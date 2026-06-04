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

#define VFS_O_RDONLY 0x000
#define VFS_O_WRONLY 0x001
#define VFS_O_RDWR 0x002
#define VFS_O_CREAT 0x040
#define VFS_O_EXCL 0x080
#define VFS_O_TRUNC 0x200
#define VFS_O_APPEND 0x400

#define VFS_INODE_REG 0x8000
#define VFS_INODE_DIR 0x4000

#define VFS_SIZE_ERR -1
#define VFS_SIZE_ZERO 0

typedef int64 vfs_size;
typedef uint64_t vfs_return_flag;
typedef uint8_t vfs_fd_t;
typedef uint32_t vfs_flags_t;
typedef uint32_t vfs_mode_t;

typedef struct vfs_ops vfs_ops_t;
typedef struct vfs_inode vfs_inode_t;
typedef struct file_descriptor vfs_file_descriptor_t;
typedef struct vfs_dentry vfs_dentry_t;

typedef struct vfs_path
{
    char path[VFS_MAX_DEPTH][VFS_MAX_FILE_FOLDER_NAME];
    uint32_t depth;
} vfs_path_t;


/////////////////////////////////////////////////////////
// INODE

typedef struct vfs_inode
{
    uint64_t ino;
    uint16_t mode;
    size_t size;
    vfs_ops_t* ops;
} vfs_inode_t;

/// FILE DESCRIPTOR
typedef struct file_descriptor
{
    vfs_inode_t *inode;
    uint64_t pos;
    vfs_flags_t flags;
} vfs_file_descriptor_t;

typedef struct vfs_ops
{
    vfs_return_flag (*open)(const char path[VFS_PATH_MAX], vfs_flags_t flags, vfs_file_descriptor_t *fd);
    vfs_return_flag (*mkdir)(const char path[VFS_PATH_MAX]);
} vfs_ops_t;

vfs_return_flag vfs_open(char path[VFS_PATH_MAX], vfs_flags_t flags, vfs_file_descriptor_t **out);
vfs_return_flag vfs_aux_parse_path(char path[VFS_PATH_MAX], vfs_path_t *out);


// DENTRY
typedef struct vfs_dentry
{
    char *name;
    struct vfs_dentry *children[VFS_MAX_CHILD];
    uint32_t child_count;
    struct vfs_dentry *parent;
    vfs_inode_t *inode;
} vfs_dentry_t;

vfs_return_flag vfs_dentry_get_child(const vfs_dentry_t *parent, const char *name, vfs_dentry_t **out);
vfs_return_flag vfs_dentry_put_child(vfs_dentry_t *parent, vfs_dentry_t *child);

/////////////////////////////////////////////////////////