#include "vfs.h"
#include "string.h"

vfs_return_flag vfs_dentry_get_child(const vfs_dentry_t *parent, const char *name, vfs_dentry_t **out)
{
    for (uint32_t i = 0; i < parent->child_count; i++)
    {
        if (strcmp(parent->children[i]->name, name) == 0)
        {
            *out = parent->children[i];
            return VFS_OK;
        }
    }
    return VFS_ENOENT;
}

vfs_return_flag vfs_dentry_put_child(vfs_dentry_t *parent, vfs_dentry_t *child)
{
    if (parent->child_count >= VFS_MAX_CHILD)
        return VFS_ENOSPC;

    // check if exists
    for (uint32_t i = 0; i < parent->child_count; i++)
    {
        if (strcmp(parent->children[i]->name, child->name) == 0)
            return VFS_EEXIST;
    }

    parent->children[parent->child_count++] = child;
    return VFS_OK;
}