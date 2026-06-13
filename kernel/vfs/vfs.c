#include "vfs.h"
#include "debug.h"
#include "kmalloc.h"
#include "kstring.h"

void vfs_init()
{
    k_log("[VFS] Initializing VFS");
    vfs_inode_count = 1;
    vfs_inode_t *vfs_root_inode = kmalloc(sizeof(vfs_inode_t));
    vfs_root.inode = vfs_root_inode;
    vfs_root.name = "ROOT";
    vfs_root.parent = &vfs_root;
    vfs_root.inode->ino = vfs_inode_count++;
    vfs_root.inode->mode = VFS_INODE_DIR;
    vfs_root.inode->ops = NULL;
    vfs_root.inode->private_field = NULL;
    k_log("[VFS] Initialized");
}

vfs_return_flag vfs_walk_parent(vfs_path_t *parsed_path, vfs_dentry_t **out)
{
    vfs_dentry_t *dir = &vfs_root;
    vfs_return_flag res;
    for (uint32_t i = 0; i < parsed_path->depth - 1; i++)
    {
        if (!dir || !dir->inode)
        {
            k_log("[VFS] Walk invalid dir");
            return VFS_EINVAL;
        }
        if (!dir->inode->ops || !dir->inode->ops->lookup)
        {
            k_log("[VFS] No valid filesystem mounted at path");
            return VFS_EINVAL;
        }
        if ((res = dir->inode->ops->lookup(dir->inode, parsed_path->path[i], &dir)) != VFS_OK)
            return res;
    }
    *out = dir;
    return VFS_OK;
}

vfs_return_flag vfs_mkdir(char path[VFS_PATH_MAX])
{
    k_log("[VFS] mkdir path %s", path);
    vfs_path_t parsed_path;
    vfs_return_flag res;
    if ((res = vfs_aux_parse_path(path, &parsed_path)) != VFS_OK)
        return res;

    // trying to create root at root
    if (parsed_path.depth == 0)
        return VFS_EEXIST;

    vfs_dentry_t *dir = NULL;
    if ((res = vfs_walk_parent(&parsed_path, &dir)) != VFS_OK)
        return res;

    if (!dir)
        return VFS_EINVAL;

    char *dirname = parsed_path.path[parsed_path.depth - 1];
    vfs_dentry_t *leaf = NULL;
    if ((res = dir->inode->ops->lookup(dir->inode, dirname, &leaf)) == VFS_OK)
    {
        k_log("[VFS] Path exists %s", leaf->name);
        return VFS_EEXIST;
    }

    k_log("[VFS] Creating new dir");
    vfs_dentry_t *new_dentry = (vfs_dentry_t *)kmalloc(sizeof(vfs_dentry_t));
    vfs_inode_t *new_inode = (vfs_inode_t *)kmalloc(sizeof(vfs_inode_t));

    new_inode->ino = vfs_inode_count++;
    new_inode->mode = VFS_INODE_DIR;
    new_inode->size = 0;
    new_inode->private_field = NULL;
    new_inode->ops = NULL;

    new_dentry->name = kstrdup(dirname);
    new_dentry->parent = dir;
    new_dentry->inode = new_inode;

    res = dir->inode->ops->mkdir(
        dir->inode,
        new_dentry);

    if (!new_inode->private_field || !new_inode->ops || res != VFS_OK)
    {
        k_log("[VFS] fs failed to create dir");
        kfree(new_dentry);
        kfree(new_inode);
        return res;
    }

    return VFS_OK;
}

vfs_return_flag vfs_open(char path[VFS_PATH_MAX], vfs_flags_t flags, vfs_file_descriptor_t **out)
{
    k_log("[VFS] Opening file %s", path);
    vfs_path_t parsed_path;
    vfs_return_flag res;

    if ((res = vfs_aux_parse_path(path, &parsed_path)) != VFS_OK)
    {
        return res;
    }

    // open root
    if (parsed_path.depth == 0)
    {
        *out = kmalloc(sizeof(vfs_file_descriptor_t));
        (*out)->flags = flags;
        (*out)->inode = vfs_root.inode;
        (*out)->pos = 0;
        (*out)->dentry = &vfs_root;
        return VFS_OK;
    }

    vfs_dentry_t *dir = NULL;
    if ((res = vfs_walk_parent(&parsed_path, &dir)) != VFS_OK)
        return res;

    if (!dir)
        return VFS_EINVAL;

    char *filename = parsed_path.path[parsed_path.depth - 1];
    vfs_dentry_t *file = NULL;
    if ((res = dir->inode->ops->lookup(dir->inode, filename, &file)) != VFS_OK)
    {
        // file does not exists
        if ((flags & VFS_O_CREAT) == 0)
            return VFS_ENOENT;

        vfs_dentry_t *new_dentry = (vfs_dentry_t *)kmalloc(sizeof(vfs_dentry_t));
        vfs_inode_t *new_inode = (vfs_inode_t *)kmalloc(sizeof(vfs_inode_t));

        new_inode->ino = vfs_inode_count++;
        new_inode->mode = VFS_INODE_REG;
        new_inode->size = 0;
        new_inode->ops = NULL;
        new_inode->private_field = NULL;

        new_dentry->name = kstrdup(filename);
        new_dentry->parent = dir;
        new_dentry->inode = new_inode;

        // parent dirs ops
        res = dir->inode->ops->open(dir->inode, new_dentry, flags);
        if (!new_inode->private_field || !new_inode->ops || res != VFS_OK)
        {
            k_log("[VFS] fs failed to create dir");
            kfree(new_dentry);
            kfree(new_inode);
            return res ? res : VFS_EINVAL;
        }

        *out = kmalloc(sizeof(vfs_file_descriptor_t));
        (*out)->flags = flags;
        (*out)->inode = new_inode;
        (*out)->pos = 0;
        (*out)->dentry = new_dentry;
        return VFS_OK;
    }

    // should never be the case
    k_log("[VFS] opening existing file!");
    if (!file)
        return VFS_EINVAL;

    // file exists
    if ((flags & VFS_O_CREAT) && (flags & VFS_O_EXCL))
        return VFS_EEXIST;

    if ((flags & VFS_O_TRUNC) != 0)
        file->inode->size = 0;

    res = file->inode->ops->open(dir->inode, file, flags);
    if (res != VFS_OK)
        return res;

    (*out) = kmalloc(sizeof(vfs_file_descriptor_t));
    (*out)->dentry = file;
    (*out)->inode = file->inode;
    (*out)->pos = 0;
    (*out)->flags = flags;

    return VFS_OK;
}

vfs_size vfs_read(vfs_file_descriptor_t *fd, uint64_t offset, uint64_t limit, void *buffer)
{
    if (!fd || !fd->inode || !fd->inode->ops || !fd->inode->ops->read || !buffer)
        return VFS_SIZE_ERR;

    // todo check fd->flags is not WR_ONLY
    vfs_size res = fd->inode->ops->read(fd->inode, offset, limit, buffer);
    return res;
}

vfs_size vfs_write(vfs_file_descriptor_t *fd, uint64_t offset, uint64_t limit, void *buffer)
{
    if (!fd || !fd->inode || !fd->inode->ops || !fd->inode->ops->write || !buffer)
        return VFS_SIZE_ERR;

    uint64_t new_limit = offset + limit;

    // overflow
    if (new_limit < offset)
        return VFS_SIZE_ERR;

    // // todo permission checks
    vfs_size res = fd->inode->ops->write(fd->inode, offset, limit, buffer);
    return res;
}

vfs_return_flag vfs_stat(vfs_file_descriptor_t *fd, vfs_stat_t **out)
{
    *out = kmalloc(sizeof(vfs_stat_t));
    (*out)->st_ino = fd->inode->ino;
    (*out)->st_mode = fd->inode->mode;
    (*out)->st_size = fd->inode->size;
}

vfs_size vfs_readdir(vfs_file_descriptor_t *fd, uint64_t offset, uint64_t size, void *buffer)
{
    if (!fd || !size || !buffer || !fd->inode || !fd->inode->ops || !fd->inode->ops->readdir)
        return VFS_SIZE_ERR;

    if (fd->inode->mode != VFS_INODE_DIR)
        return VFS_SIZE_ERR;

    vfs_size res = fd->inode->ops->readdir(fd->inode, offset, size, buffer);
    return res;
}

vfs_return_flag vfs_close(vfs_file_descriptor_t *fd)
{
    if (!fd || !fd->inode || !fd->inode->ops || !fd->inode->ops->close)
    {
        return VFS_EINVAL;
    }
    // todo check reference counts and kfree
    fd->inode->ops->close(fd->inode);
    return VFS_OK;
}

vfs_return_flag vfs_validate_ops(vfs_ops_t *ops)
{
    if (!ops)
        return VFS_EINVAL;

    if (!ops ||
        !ops->lookup ||
        !ops->mkdir ||
        !ops->open ||
        !ops->read ||
        !ops->write ||
        !ops->readdir ||
        !ops->close ||
        !ops->fs_driver_name)
        return VFS_EINVAL;

    return VFS_OK;
}

vfs_return_flag vfs_mount(char path[VFS_PATH_MAX], vfs_superblock_t *sb)
{
    if (!sb || !sb->fs_ops) {
        k_log("[VFS] Superblock invalid");
        return VFS_EINVAL;
    }
    
    k_log("[VFS] mounting file system on %s driver %s", path, sb->fs_ops->fs_driver_name);
    vfs_path_t parsed_path;
    vfs_return_flag res;

    res = vfs_validate_ops(sb->fs_ops);
    if (res != VFS_OK)
    {
        k_log("[VFS] Invalid filesystem implementation");
        return res;
    }

    res = vfs_aux_parse_path(path, &parsed_path);
    if (res != VFS_OK)
        return res;

    if (!path)
    {
        k_log("[VFS] Invalid path to mount");
        return VFS_EINVAL;
    }

    if (parsed_path.depth == 0)
    {
        k_log("[VFS] mounting at root");
        if (vfs_root.inode->ops && vfs_root.inode->private_field)
        {
            k_log("[VFS] root can be only mounted once");
            return VFS_EINVAL;
        }
        k_log("[VFS] goinf to mount");
        vfs_root.inode->ops = sb->fs_ops;
        vfs_root.inode->private_field = sb->private_fields;
        return VFS_OK;
    }
}