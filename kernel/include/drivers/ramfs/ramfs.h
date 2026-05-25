#include "vfs.h"
#include "kmalloc.h"

vfs_ops_t* ramfs_init();

vfs_return_flag ramfs_open(char *path, vfs_flags_t flags, vfs_file_descriptor_t**out);