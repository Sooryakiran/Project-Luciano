#pragma once
#include "vfs.h"
#include "kmalloc.h"

#define RAMDISK_MAX_FILES 4096
#define RAMFS_RESERVED_SUB 2
#define RAMDISK_INITIAL_FILE_ALLOC 16
#define RAMFS_MAX_CHILD 1024
#define RAMFS_DRIVER_NAME "ramfs"

void ramfs_init();

vfs_return_flag ramfs_lookup(vfs_inode_t *inode, const char *name, vfs_dentry_t **out);
vfs_return_flag ramfs_mkdir(vfs_inode_t *parent, vfs_dentry_t *new_dir);
vfs_return_flag ramfs_open(vfs_inode_t *parent, vfs_dentry_t *new_dentry, vfs_flags_t flags);
vfs_size ramfs_read(vfs_inode_t *inode, uint64_t offset, uint64_t limit, void *buffer);
vfs_size ramfs_write(vfs_inode_t *inode, uint64_t offset, uint64_t limit, void *buffer);
vfs_size ramfs_readdir(vfs_inode_t *inode, uint64_t offset, uint64_t size, void *buffer);
vfs_return_flag ramfs_close(vfs_inode_t *inode);

uint16_t ramfs_emit_entry(void *buf, uint64_t limit, vfs_inode_t *inode, const char *entry_name);
vfs_return_flag ramfs_putchild(vfs_inode_t *inode, vfs_dentry_t *child);

vfs_superblock_t *ramfs_get_sb();
vfs_ops_t *ramfs_get_mount_ops();
void *ramfs_get_mount_private_fields();