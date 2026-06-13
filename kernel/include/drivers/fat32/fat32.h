#pragma once
#include "vfs.h"

vfs_return_flag fat32_lookup(vfs_inode_t *inode, const char *name, vfs_dentry_t **out);
vfs_return_flag fat32_mkdir(vfs_inode_t *parent, vfs_dentry_t *new_dir);