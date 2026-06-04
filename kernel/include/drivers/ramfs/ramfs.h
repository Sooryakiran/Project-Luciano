#include "vfs.h"
#include "kmalloc.h"

vfs_ops_t* ramfs_init();

vfs_return_flag ramfs_open(char *path, vfs_flags_t flags, vfs_file_descriptor_t**out);
vfs_return_flag ramfs_mkdir(char *path);
vfs_size ramfs_read(vfs_file_descriptor_t* fd, uint64_t offset, uint64_t limit, void *buffer);
vfs_size ramfs_write(vfs_file_descriptor_t* fd, uint64_t offset, uint64_t limit, void *buffer);