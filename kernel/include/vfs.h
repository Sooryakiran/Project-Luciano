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
#define VFS_ERROR 42
#define VFS_ENOMEM 12
#define VFS_EINVAL 22

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
typedef struct vfs_stat vfs_stat_t;


static vfs_dentry_t vfs_root;
static uint32_t vfs_inode_count = 1;

typedef struct vfs_path
{
    char path[VFS_MAX_DEPTH][VFS_MAX_FILE_FOLDER_NAME];
    uint32_t depth;
} vfs_path_t;

typedef struct vfs_inode
{
    uint64_t ino;
    uint16_t mode; // here mode is only type, not linux mode with perms
    size_t size;
    vfs_ops_t *ops;
    void *private_field;
} vfs_inode_t;

typedef struct file_descriptor
{
    vfs_inode_t *inode;
    vfs_dentry_t *dentry;
    uint64_t pos;
    vfs_flags_t flags;
} vfs_file_descriptor_t;

typedef struct vfs_ops
{
    vfs_return_flag (*lookup)(vfs_inode_t *inode, const char *name, vfs_dentry_t **out);
    vfs_return_flag (*mkdir)(vfs_inode_t *parent, vfs_dentry_t *new_dir);
    vfs_return_flag (*open)(vfs_inode_t *parent, vfs_dentry_t *new_dentry, vfs_flags_t flags);
    vfs_size (*read)(vfs_inode_t *inode, uint64_t offset, uint64_t limit, void *buffer);
    vfs_size (*write)(vfs_inode_t *inode, uint64_t offset, uint64_t limit, void *buffer);
    vfs_size (*readdir)(vfs_inode_t *inode, uint64_t offset, uint64_t size, void *buffer);
    vfs_return_flag (*close)(vfs_inode_t *inode);
    const char *fs_driver_name;
} vfs_ops_t;

typedef struct vfs_dentry
{
    char *name;
    struct vfs_dentry *parent;
    vfs_inode_t *inode;
} vfs_dentry_t;

typedef struct vfs_stat
{
    size_t st_size; // 1 word put first, don't put middle
    uint32_t st_ino;
    uint16_t st_mode;
} vfs_stat_t;

typedef struct vfs_dir_entry
{
    uint64_t ino;
    uint16_t type;
    uint16_t name_len;
    uint16_t rec_len;
    uint16_t _pad; // just to make sure name is alligned
    char name[];
} vfs_dir_entry_t;

void vfs_init();

vfs_return_flag vfs_mkdir(char path[VFS_PATH_MAX]);
vfs_return_flag vfs_open(char path[VFS_PATH_MAX], vfs_flags_t flags, vfs_file_descriptor_t **out);
vfs_size vfs_read(vfs_file_descriptor_t *fd, uint64_t offset, uint64_t limit, void *buffer);
vfs_size vfs_write(vfs_file_descriptor_t *fd, uint64_t offset, uint64_t limit, void *buffer);
vfs_return_flag vfs_stat(vfs_file_descriptor_t *fd, vfs_stat_t **out);
vfs_size vfs_readdir(vfs_file_descriptor_t *fd, uint64_t offset, uint64_t size, void *buffer);
vfs_return_flag vfs_close(vfs_file_descriptor_t *fd);

vfs_return_flag vfs_mount(char path[VFS_PATH_MAX], vfs_ops_t *ops, void *private_fields);
vfs_return_flag vfs_aux_parse_path(char path[VFS_PATH_MAX], vfs_path_t *out);
vfs_return_flag vfs_validate_ops(vfs_ops_t *ops);