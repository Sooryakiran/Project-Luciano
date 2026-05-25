#include "arch/x86_64/syscall.h"
#include "types.h"
#include "vfs.h"
#include "debug.h"
#include "kmalloc.h"

uint64_t sys_vfs_open(uint64_t arg1, uint64_t arg2, uint64_t arg3) {
    char *file_path = (char *)arg1;
    vfs_flags_t flags = (vfs_flags_t)arg2;
    vfs_mode_t mode = (vfs_mode_t)arg3;

    // vfs_file_descriptor_t *fd = kmalloc(sizeof(vfs_file_descriptor_t));
    // vfs_return_flag res = vfs_open(file_path, flags, &fd);
    return 0;
}