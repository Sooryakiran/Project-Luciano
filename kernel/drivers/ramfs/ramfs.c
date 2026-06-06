#include "drivers/ramfs/ramfs.h"
#include "vfs.h"
#include "kstring.h"
#include "debug.h"
#include "string.h"
#include "kbuf.h"

#define RAMDISK_MAX_FILES 4096
#define RAMFS_RESERVED_SUB 2
#define RAMDISK_INITIAL_FILE_ALLOC 16
#define RAMFS_BYTE 8

typedef kbuf_t ramfs_file_t;

static ramfs_file_t data[RAMDISK_MAX_FILES];
static vfs_ops_t ops = {};

static vfs_inode_t root_inode = {
    .ino = 1,
    .mode = VFS_INODE_DIR,
    .ops = &ops,
    .size = 0,
};

static vfs_dentry_t root = {
    .child_count = 0,
    .children = {},
    .inode = NULL};

static uint32_t ramfs_file_count = 1;
static uint32_t ramfs_dir_count = 2;

vfs_inode_t *ramfs_create_file()
{
    ramfs_file_t file;
    file.data = kmalloc(sizeof(char) * RAMDISK_INITIAL_FILE_ALLOC);
    file.allocated = RAMDISK_INITIAL_FILE_ALLOC;

    vfs_inode_t *node = kmalloc(sizeof(vfs_inode_t));
    node->ino = ramfs_file_count;
    node->mode = VFS_INODE_REG;
    node->size = 0;
    node->ops = &ops;

    data[ramfs_file_count++] = file;

    return node;
}

vfs_inode_t *ramfs_create_directory()
{
    vfs_inode_t *node = kmalloc(sizeof(vfs_inode_t));
    node->ino = ramfs_dir_count;
    node->mode = VFS_INODE_DIR;
    node->size = 0;
    node->ops = &ops;
    return node;
}

typedef char *file;

vfs_ops_t *ramfs_init()
{
    root = (vfs_dentry_t){
        .child_count = 0,
        .name = "ROOT",
        .children = {},
        .inode = NULL};

    root.parent = &root;
    root.inode = &root_inode;

    return &ops;
}

vfs_return_flag ramfs_mkdir(char *path)
{
    k_log("[RAMFS] mkdir path %s", path);
    vfs_path_t parsed_path;
    vfs_return_flag res;
    if ((res = vfs_aux_parse_path(path, &parsed_path)) != VFS_OK)
    {
        return res;
    }

    // walk til parent
    vfs_dentry_t *dir = &root;
    for (uint32_t i = 0; i < parsed_path.depth - 1; i++)
    {
        if ((res = vfs_dentry_get_child(dir, parsed_path.path[i], &dir)) != VFS_OK)
        {
            return VFS_ENOENT;
        }
    }

    vfs_dentry_t *leaf = NULL;
    if ((res = vfs_dentry_get_child(dir, parsed_path.path[parsed_path.depth - 1], &leaf)) != VFS_OK)
    {
        k_log("[RAMFS] creating new dir");
        vfs_inode_t *dir_node = ramfs_create_directory(kstrdup(parsed_path.path[parsed_path.depth - 1]));

        vfs_dentry_t *new_dentry = (vfs_dentry_t *)kmalloc(sizeof(vfs_dentry_t));
        new_dentry->inode = dir_node;
        new_dentry->name = kstrdup(parsed_path.path[parsed_path.depth - 1]);
        new_dentry->child_count = 0;

        vfs_dentry_put_child(dir, new_dentry);

        return VFS_OK;
    }
    else
    {
        k_log("[RAMFS] Path exists %s", leaf->name);
        return VFS_EEXIST;
    }
}

vfs_return_flag ramfs_open(char *path, vfs_flags_t flags, vfs_file_descriptor_t **out)
{
    k_log("[RAMFS] Opening file %s", path);
    vfs_path_t parsed_path;
    vfs_return_flag res;
    if ((res = vfs_aux_parse_path(path, &parsed_path)) != VFS_OK)
    {
        return res;
    }

    // now we are at parent directory
    vfs_dentry_t *file = NULL;
    if (parsed_path.depth == 0) {
        // handle root case
        *out = kmalloc(sizeof(vfs_file_descriptor_t));
        (*out)->flags = flags;
        (*out)->inode = &root_inode;
        (*out)->pos = 0;
        (*out)->dentry = &root;
        return VFS_OK;
    }

    // walk till parent directory
    vfs_dentry_t *dir = &root;
    for (uint32_t i = 0; i < parsed_path.depth - 1; i++)
    {
        k_log("LOOPING\n");
        if ((res = vfs_dentry_get_child(dir, parsed_path.path[i], &dir)) != VFS_OK)
        {
            return VFS_ENOENT;
        }
    }

    if ((res = vfs_dentry_get_child(dir, parsed_path.path[parsed_path.depth - 1], &file)) != VFS_OK)
    {
        k_log("[RAMFS] File does not exist");
        // no exist child
        if ((flags & VFS_O_CREAT) == 0)
        {
            return VFS_ENOENT;
        }

        // does not exist, create new
        vfs_inode_t *new_file_node = ramfs_create_file();
        *out = kmalloc(sizeof(vfs_file_descriptor_t));
        (*out)->inode = new_file_node;
        (*out)->pos = 0;
        (*out)->flags = flags;

        // create child entry
        vfs_dentry_t *new_dentry = (vfs_dentry_t *)kmalloc(sizeof(vfs_dentry_t));
        new_dentry->inode = new_file_node;
        new_dentry->name = kstrdup(parsed_path.path[parsed_path.depth - 1]);
        k_log("[RAMFS] Creating new file %s", parsed_path.path[parsed_path.depth - 1]);
        new_dentry->child_count = 0;
        vfs_dentry_put_child(dir, new_dentry);
        (*out)->dentry = new_dentry;

        return VFS_OK;
    }
    else
    {
        k_log("[RAMFS] File exists!");
        if ((flags & VFS_O_CREAT) && (flags & VFS_O_EXCL))
        {
            return VFS_EEXIST;
        }

        if ((flags & VFS_O_TRUNC) != 0)
        {
            file->inode->size = 0;
        }

        *out = kmalloc(sizeof(vfs_file_descriptor_t));
        (*out)->flags = flags;
        (*out)->inode = file->inode;
        (*out)->pos = 0;
        (*out)->dentry = file;
        return VFS_OK;
    }

    return 0;
}

vfs_size ramfs_read(vfs_file_descriptor_t *fd, uint64_t offset, uint64_t limit, void *buffer)
{
    if (!fd || !buffer)
        return VFS_SIZE_ERR;

    if (fd->inode->mode != VFS_INODE_REG)
        return VFS_SIZE_ERR;

    if (offset >= fd->inode->size)
        return VFS_SIZE_ZERO;

    uint64_t size_limit = fd->inode->size - offset;
    if (limit < size_limit)
        size_limit = limit;

    memcpy(buffer, data[fd->inode->ino].data + offset * sizeof(char), size_limit);

    return size_limit;
}

vfs_size ramfs_write(vfs_file_descriptor_t *fd, uint64_t offset, uint64_t limit, void *buffer)
{
    if (!fd || !buffer)
        return VFS_SIZE_ERR;

    if (fd->inode->mode != VFS_INODE_REG)
        return VFS_SIZE_ERR;

    uint64_t new_limit = offset + limit;

    if (new_limit < offset)
        return VFS_SIZE_ERR;

    uint64_t ino = fd->inode->ino;

    while (new_limit > data[ino].allocated)
    {
        if (kbuf_grow(&(data[ino])) != K_STATUS_OK)
        {
            return VFS_SIZE_ERR;
        }
    }

    // update size
    if (new_limit > fd->inode->size)
    {
        fd->inode->size = new_limit;
    }

    // now write here
    memcpy(data[ino].data + sizeof(char) * offset, buffer, limit);
    return limit;
}

vfs_return_flag ramfs_stat(vfs_file_descriptor_t *fd, vfs_stat_t **out)
{
    *out = kmalloc(sizeof(vfs_stat_t));
    (*out)->st_ino = fd->inode->ino;
    (*out)->st_mode = fd->inode->mode;
    (*out)->st_size = fd->inode->size;
    return VFS_OK;
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
    memcpy(entry->name, entry_name, name_len);
    entry->name[name_len] = '\0';
    return size;
}

vfs_size ramfs_readdir(vfs_file_descriptor_t *fd, uint64_t offset, uint64_t size, void *buffer)
{
    if (!size || !buffer)
        return VFS_SIZE_ERR;

    if (fd->inode->mode != VFS_INODE_DIR)
        return VFS_SIZE_ERR;

    uint32_t total_entries = RAMFS_RESERVED_SUB + fd->dentry->child_count;

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
            size_written = ramfs_emit_entry(buffer_p, size, fd->inode, ".");
            break;
        case 1:
            size_written = ramfs_emit_entry(buffer_p, size, fd->dentry->parent->inode, "..");
            break;
        default:
            vfs_dentry_t *dentry = fd->dentry->children[offset - RAMFS_RESERVED_SUB];
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