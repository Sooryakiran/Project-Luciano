#include "drivers/ramfs/ramfs.h"
#include "vfs.h"
#include "kstring.h"
#include "debug.h"
#include "string.h"
#include "kbuf.h"

#define offsetof(t, m) __builtin_offsetof(t, m)

#define RAMFS_BYTE 8


typedef kbuf_t ramfs_file_t;

typedef struct ramfs_private_inode
{
    uint64_t file_index;
    struct vfs_dentry *children[VFS_MAX_CHILD];
    uint32_t child_count;
    struct vfs_inode *parent;
} ramfs_private_inode_t;

static ramfs_file_t data[RAMDISK_MAX_FILES];

static vfs_ops_t ops = {
    .lookup = ramfs_lookup,
    .mkdir = ramfs_mkdir,
    .open = ramfs_open,
    .read = ramfs_read,
    .write = ramfs_write,
    .readdir = ramfs_readdir,
    .close = ramfs_close,
    .fs_driver_name = RAMFS_DRIVER_NAME};

static vfs_inode_t root_inode = {
    .ino = 1,
    .mode = VFS_INODE_DIR,
    .ops = &ops,
    .size = 0,
};

static uint32_t ramfs_file_count = 1;
static uint32_t ramfs_dir_count = 2;

typedef char *file;

void ramfs_init()
{
    for (uint32_t i = 0; i < RAMDISK_MAX_FILES; i++)
    {
        kbuf_init(&data[i], RAMDISK_INITIAL_FILE_ALLOC);
    }
}

vfs_ops_t *ramfs_get_mount_ops()
{
    return &ops;
}

void *ramfs_get_mount_private_fields()
{
    ramfs_private_inode_t *private_field = kmalloc(sizeof(ramfs_private_inode_t));
    private_field->child_count = 0;
    private_field->file_index = 0;
    private_field->parent = NULL;
    return private_field;
}

vfs_return_flag ramfs_mkdir(vfs_inode_t *parent_inode, vfs_dentry_t *new_dir)
{
    if (!parent_inode || !new_dir || !new_dir->inode)
        return VFS_EINVAL;

    if (parent_inode->mode != VFS_INODE_DIR)
        return VFS_EINVAL;

    ramfs_private_inode_t *private_field = kmalloc(sizeof(ramfs_private_inode_t));
    if (!private_field)
        return VFS_ENOMEM;

    private_field->child_count = 0;
    private_field->file_index = 0;
    private_field->parent = parent_inode;

    new_dir->inode->ops = &ops;
    new_dir->inode->private_field = private_field;

    vfs_return_flag res = ramfs_putchild(parent_inode, new_dir);
    if (res != VFS_OK)
    {
        kfree(private_field);
        new_dir->inode->private_field = NULL;
        return res;
    }

    return VFS_OK;
}

vfs_return_flag ramfs_open(vfs_inode_t *parent_inode, vfs_dentry_t *new_dentry, vfs_flags_t flags)
{
    k_log("[RAMFS] Opening file");

    if (!parent_inode || !new_dentry || !new_dentry->inode)
        return VFS_EINVAL;

    if (flags & VFS_O_CREAT)
    {
        ramfs_private_inode_t *private_field = (ramfs_private_inode_t *)kmalloc(sizeof(ramfs_private_inode_t));
        if (!private_field)
            return VFS_ENOMEM;
        private_field->file_index = ramfs_file_count++;
        private_field->child_count = 0;
        private_field->parent = parent_inode;

        new_dentry->inode->ops = &ops;
        new_dentry->inode->private_field = private_field;

        // attach to parent dentry
        vfs_return_flag res = ramfs_putchild(parent_inode, new_dentry);
        if (res != VFS_OK)
        {
            kfree(private_field);
            new_dentry->inode->private_field = NULL;
            return res;
        }

        return VFS_OK;
    }

    // existing file, basically a no op here
    if (new_dentry->inode->ops == NULL || new_dentry->inode->private_field == NULL)
        return VFS_EINVAL;

    return VFS_OK;
}

vfs_size ramfs_read(vfs_inode_t *inode, uint64_t offset, uint64_t limit, void *buffer)
{
    k_log("[RAMFS] Read all validating...");
    if (!inode || !buffer)
    {
        k_log("[RAMFS] Read got invalid inode or buffer");
        return VFS_SIZE_ERR;
    }

    if (inode->mode != VFS_INODE_REG)
    {
        k_log("[RAMFS] Read got invalid inode type");
        return VFS_SIZE_ERR;
    }

    if (offset >= inode->size)
        return VFS_SIZE_ZERO;

    k_log("[RAMFS] Read all validations passed");
    uint64_t size_limit = inode->size - offset;
    if (limit < size_limit)
        size_limit = limit;

    ramfs_private_inode_t *private_node = (ramfs_private_inode_t *)inode->private_field;

    if (!private_node)
        return VFS_SIZE_ERR;

    memcpy(buffer, data[private_node->file_index].data + offset, size_limit);

    return size_limit;
}

vfs_size ramfs_write(vfs_inode_t *inode, uint64_t offset, uint64_t limit, void *buffer)
{
    k_log("[RAMFS] Writing file");
    if (!inode || !buffer)
        return VFS_SIZE_ERR;

    if (inode->mode != VFS_INODE_REG)
        return VFS_SIZE_ERR;

    uint64_t new_limit = offset + limit;

    if (new_limit < offset)
        return VFS_SIZE_ERR;

    k_log("[RAMFS] Writing file all validations completed!");
    ramfs_private_inode_t *private_node = (ramfs_private_inode_t *)inode->private_field;

    if (!private_node)
        return VFS_SIZE_ERR;

    k_log("[RAMFS] Writing private node intact!");
    while (new_limit > data[private_node->file_index].allocated)
    {
        k_log("[RAMFS] Reallocating ramdisk buffer for file %d, (current size: %d, required size %d)", private_node->file_index, data[private_node->file_index].allocated, new_limit);
        if (kbuf_grow(&(data[private_node->file_index])) != K_STATUS_OK)
        {
            return VFS_SIZE_ERR;
        }
    }

    // update size
    if (new_limit > inode->size)
    {
        inode->size = new_limit;
    }

    // now write here
    memcpy(data[private_node->file_index].data + offset, buffer, limit);
    return limit;
}

uint16_t ramfs_emit_entry(void *buf, uint64_t limit, vfs_inode_t *inode, const char *entry_name)
{
    uint16_t name_len = strlen(entry_name);
    uint16_t size = offsetof(vfs_dir_entry_t, name) + name_len + 1;

    // allign
    size = (size + RAMFS_BYTE - 1) & ~(RAMFS_BYTE - 1);
    k_log("[RAMFS] emit size is %d, limit is %d", size, limit);
    if (size > limit)
        return 0;

    vfs_dir_entry_t *entry = buf;
    entry->ino = inode->ino;
    entry->type = inode->mode;
    entry->name_len = name_len;
    entry->rec_len = size;
    memcpy(entry->name, (void *)entry_name, name_len);
    entry->name[name_len] = '\0';
    return size;
}

vfs_size ramfs_readdir(vfs_inode_t *inode, uint64_t offset, uint64_t size, void *buffer)
{
    if (!size || !buffer || !inode)
        return VFS_SIZE_ERR;

    if (inode->mode != VFS_INODE_DIR)
        return VFS_SIZE_ERR;

    ramfs_private_inode_t *private_fields = (ramfs_private_inode_t *)inode->private_field;
    if (!private_fields)
        return VFS_SIZE_ERR;

    uint32_t total_entries = RAMFS_RESERVED_SUB + private_fields->child_count;

    if (offset >= total_entries)
        return VFS_SIZE_ZERO;

    vfs_size curr_size = 0;
    char *buffer_p = (char *)buffer;

    // okay now we create each
    while (offset < total_entries)
    {
        uint16_t size_written = 0;
        switch (offset)
        {
        case 0:
            size_written = ramfs_emit_entry(buffer_p, size, inode, ".");
            break;
        case 1:
            if (private_fields->parent == NULL)
                // no parent, ".." of root is "."
                size_written = ramfs_emit_entry(buffer_p, size, inode, "..");
            else
                size_written = ramfs_emit_entry(buffer_p, size, private_fields->parent, "..");
            break;
        default: ;
            vfs_dentry_t *dentry = private_fields->children[offset - RAMFS_RESERVED_SUB];
            size_written = ramfs_emit_entry(buffer_p, size, dentry->inode, dentry->name);
            break;
        }

        k_log("[RAMFS] Read dir wrote offset %d (size %d)", offset, size_written);
        // no more space in buffer
        if (size_written == 0)
        {
            break;
        }

        // increment buffer
        buffer_p += size_written;
        size -= size_written;
        curr_size += size_written;
        offset++;
    }

    return curr_size;
}

vfs_return_flag ramfs_close(vfs_inode_t *inode)
{
    // no op here
    return VFS_OK;
}

vfs_return_flag ramfs_lookup(vfs_inode_t *inode, const char *name, vfs_dentry_t **out)
{
    ramfs_private_inode_t *private_node = (ramfs_private_inode_t *)inode->private_field;
    k_log("[RAMFS] Lookup for child %s, num kids is %d", name, private_node->child_count);

    if (inode->mode != VFS_INODE_DIR)
        return VFS_ENOENT;

    for (uint32_t i = 0; i < private_node->child_count; i++)
    {
        if (strcmp(private_node->children[i]->name, name) == 0)
        {
            k_log("[RAMFS] Lookup found match %s vs %s", name, private_node->children[i]->name);
            *out = private_node->children[i];
            return VFS_OK;
        }
        else
        {
            k_log("[RAMFS] no match %s vs %s", name, private_node->children[i]->name);
        }
    }
    k_log("[RAMFS] lookup failed");
    return VFS_ENOENT;
}

vfs_return_flag ramfs_putchild(vfs_inode_t *inode, vfs_dentry_t *child)
{
    k_log("[RAMFS] Put new child %s", child->name);
    ramfs_private_inode_t *private_node = (ramfs_private_inode_t *)inode->private_field;
    if (private_node->child_count >= RAMFS_MAX_CHILD)
        return VFS_ENOSPC;

    // if (inode->mode != VFS_INODE_DIR)
    //     return VFS_ENOENT;

    for (uint32_t i = 0; i < private_node->child_count; i++)
    {
        if (strcmp(private_node->children[i]->name, child->name) == 0)
            return VFS_EEXIST;
    }

    private_node->children[private_node->child_count++] = child;
    return VFS_OK;
}

vfs_superblock_t *ramfs_get_sb() {
    vfs_superblock_t *sb = (vfs_superblock_t *)kmalloc(sizeof(vfs_superblock_t));
    sb->volume = NULL;
    sb->fs_ops = ramfs_get_mount_ops();
    sb->private_fields = ramfs_get_mount_private_fields();
    return sb;
}