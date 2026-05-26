#pragma once
#include "types.h"

#define VFS_MAX_FILE_FOLDER_NAME 256               // max file or folder name size
#define VFS_PATH_MAX 4096                          // max path length
#define VFS_MAX_DEPTH VFS_MAX_FILE_FOLDER_NAME / 2 // max depth of the filesystem
#define VFS_PATH_SEP_CHAR '/'
#define VFS_PATH_DOT '.'

#define VFS_MAX_CHILD 1024

#define VFS_ENAMETOOLONG 36
#define VFS_OK 0
#define VFS_ENOENT 2
#define VFS_EEXIST 17
#define VFS_ENOSPC 28

typedef uint64_t vfs_return_flag;
typedef uint8_t vfs_fd_t;
typedef uint32_t vfs_flags_t;
typedef uint32_t vfs_mode_t;

typedef struct vfs_path
{
    char path[VFS_MAX_DEPTH][VFS_MAX_FILE_FOLDER_NAME];
    uint32_t depth;
} vfs_path_t;

typedef struct vfs_inode
{

} vfs_inode_t;

typedef struct file_descriptor
{
    vfs_inode_t *inode;
    uint64_t pos;
    vfs_flags_t flags;
} vfs_file_descriptor_t;

// DENTRY
typedef struct vfs_dentry
{
    char name[VFS_MAX_FILE_FOLDER_NAME];
    struct vfs_dentry *children[VFS_MAX_CHILD];
    uint32_t child_count;
    struct vfs_dentry *parent;
} vfs_dentry_t;

vfs_return_flag vfs_dentry_get_child(const vfs_dentry_t *parent, const char *name, vfs_dentry_t **out);
vfs_return_flag vfs_dentry_put_child(vfs_dentry_t *parent, vfs_dentry_t* child);

/////////////////////////////////////////////////////////
typedef struct vfs_ops
{
    vfs_return_flag (*open)(const char path[VFS_PATH_MAX], vfs_flags_t flags, vfs_file_descriptor_t *fd);
} vfs_ops_t;

vfs_return_flag vfs_open(char path[VFS_PATH_MAX], vfs_flags_t flags, vfs_file_descriptor_t **out);
uint8_t vfs_aux_parse_path(char path[VFS_PATH_MAX], vfs_path_t *out);