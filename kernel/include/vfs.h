#pragma once
#include "types.h"

#define MAX_FILE_FOLDER_NAME 256

typedef uint64_t vfs_return_flag;
typedef uint8_t vfs_fd_t;
typedef uint32_t vfs_flags_t;
typedef uint32_t vfs_mode_t;

typedef struct vfs_inode
{

} vfs_inode_t;

typedef struct file_descriptor
{
    vfs_inode_t* inode;
    uint64_t pos;
    vfs_flags_t flags;
} vfs_file_descriptor_t;

typedef struct dentry {
    char name[MAX_FILE_FOLDER_NAME];
} dentry_t;

typedef struct vfs_ops {
    vfs_return_flag (*open)(const char *path, vfs_flags_t flags, vfs_file_descriptor_t *fd);
} vfs_ops_t;

vfs_return_flag vfs_open(char *path, vfs_flags_t flags,  vfs_file_descriptor_t **out);