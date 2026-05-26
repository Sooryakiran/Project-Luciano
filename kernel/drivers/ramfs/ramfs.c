#include "drivers/ramfs/ramfs.h"
#include "vfs.h"

static vfs_ops_t ops = {};
static vfs_dentry_t root = {};

typedef char * file;

vfs_ops_t* ramfs_init() {
    return &ops;
}

vfs_return_flag ramfs_open(char *path, vfs_flags_t flags, vfs_file_descriptor_t**out) {
    
    return 0;
}