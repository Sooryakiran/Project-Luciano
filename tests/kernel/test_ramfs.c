#include <assert.h>
#include "stdio.h"
#include "drivers/ramfs/ramfs.h"

void test_ramfs_open_read_noexists() {
    ramfs_init();
    printf("[RAMFS TEST] Testing ramfs open not exists\n");
    vfs_file_descriptor_t* result = NULL;
    vfs_return_flag res = ramfs_open("/home/soorkie", VFS_O_RDONLY, &result);
    assert(res == VFS_ENOENT);
    printf("[RAMFS TEST] Test passed!\n");
}

void test_ramfs_open_write_noexists() {
    ramfs_init();
    printf("[RAMFS TEST] Testing ramfs write not exists\n");
    vfs_file_descriptor_t* result = NULL;
    vfs_return_flag res = ramfs_open("/home/soorkie", VFS_O_WRONLY, &result);
    assert(res == VFS_ENOENT);
    printf("[RAMFS TEST] Test passed!\n");
}

void test_ramfs_open_write_with_create() {
    ramfs_init();
    printf("[RAMFS TEST] Testing ramfs write with create not exists\n");
    vfs_file_descriptor_t* result = NULL;
    vfs_return_flag res = ramfs_open("/soorkie", VFS_O_WRONLY | VFS_O_CREAT, &result);
    assert(res == VFS_OK);
    assert(result->flags == (VFS_O_WRONLY | VFS_O_CREAT));
    assert(result->inode->size == 0);
    printf("[RAMFS TEST] Test passed\n");
}

void test_ramfs_open_read_existing() {
    ramfs_init();
    printf("[RAMFS TEST] Testing ramfs read existing\n");
    vfs_file_descriptor_t* result = NULL;
    vfs_return_flag res = ramfs_open("/soorkie", VFS_O_WRONLY | VFS_O_CREAT, &result);
    assert(res == VFS_OK);
    vfs_return_flag res_2 = ramfs_open("/soorkie", VFS_O_RDONLY, &result);
    assert(res_2 == VFS_OK);
}

void test_ramfs_open_write_with_create_nested() {
    printf("[RAMFS TEST] Testing ramfs write with create not exists but nested\n");
    vfs_file_descriptor_t* result = NULL;
    vfs_return_flag res = ramfs_open("/nested/soorkie", VFS_O_WRONLY | VFS_O_CREAT, &result);
    assert(res == VFS_ENOENT);
    printf("[RAMFS TEST] Test passed\n");
}


void test_ramfs_open_write_existing_with_excl() {
    ramfs_init();
    printf("[RAMFS TEST] Testing ramfs read existing\n");
    vfs_file_descriptor_t* result = NULL;
    vfs_return_flag res = ramfs_open("/soorkie_exists", VFS_O_WRONLY | VFS_O_CREAT, &result);
    assert(res == VFS_OK);
    vfs_return_flag res_2 = ramfs_open("/soorkie_exists", VFS_O_WRONLY | VFS_O_CREAT | VFS_O_EXCL, &result);
    assert(res_2 == VFS_EEXIST);
}

void test_ramfs_mkdir() {
    ramfs_init();
    printf("[RAMFS TEST] Testing ramfs mkdir\n");

    vfs_return_flag res_0 = ramfs_mkdir("/home");
    assert(res_0 == VFS_OK);

    vfs_return_flag res_1 = ramfs_mkdir("/home");
    assert(res_1 == VFS_EEXIST);

    vfs_return_flag res_2 = ramfs_mkdir("/soorkie/images");
    assert(res_2 == VFS_ENOENT);

    vfs_return_flag res_3 = ramfs_mkdir("soorkie");
    assert(res_3 == VFS_OK);

    vfs_return_flag res_4 = ramfs_mkdir("/soorkie/images");
    assert(res_4 == VFS_OK);

    printf("[RAMFS TEST] Test passed \n");
}

int main() {
    printf("[RAMFS TEST] ramfs test suite...\n");
    test_ramfs_open_read_noexists();
    test_ramfs_open_write_noexists();
    test_ramfs_open_write_with_create();
    test_ramfs_open_write_with_create_nested();
    test_ramfs_open_read_existing();
    test_ramfs_open_write_existing_with_excl();
    test_ramfs_mkdir();
    return 0;
}