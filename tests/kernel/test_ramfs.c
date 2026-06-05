#include <assert.h>
#include "stdio.h"
#include "drivers/ramfs/ramfs.h"
#include "string.h"

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

void test_ramfs_write_and_read_normal() {
    ramfs_init();
    printf("[RAMFS TEST] Testing ramfs write and read happy path.\n");
    
    // open a file
    vfs_file_descriptor_t* result = NULL;
    vfs_return_flag res = ramfs_open("/soorkie", VFS_O_WRONLY | VFS_O_CREAT, &result);
    assert(res == VFS_OK);

    // write to that place
    char *content_to_write = "soorkie is the king!";
    vfs_size res_1 = ramfs_write(result, 0, strlen(content_to_write) + 1, content_to_write);
    assert(res_1 == strlen(content_to_write) + 1);

    // now read
    char *read_buffer = kmalloc(strlen(content_to_write) + 1);
    vfs_size res_2 = ramfs_read(result, 0, strlen(content_to_write) + 1, read_buffer);
    assert(strcmp(content_to_write, read_buffer) == 0);
    assert(res_2 == strlen(content_to_write) + 1);

    // now write to some other place
    char *second_content_to_write = "hahahaha, i am the ruler of earth";
    vfs_size res_3 = ramfs_write(result, 1024, strlen(second_content_to_write) + 1, second_content_to_write);
    
    // now read that part
    char *second_read_buffer = kmalloc(strlen(second_content_to_write) + 1);
    vfs_size res_4 = ramfs_read(result, 1024, strlen(second_content_to_write) + 1, second_read_buffer);
    assert(res_4 == strlen(second_content_to_write) + 1);
    assert(strcmp(second_content_to_write, second_read_buffer) == 0);

    // now replace king with "god!"
    char *replacer = "god!";
    vfs_size res_5 = ramfs_write(result, 15, 4, replacer);
    assert(res_5 = 4);
    char *third_read_buffer = kmalloc(strlen(content_to_write) + 1);
    vfs_size res_6 = ramfs_read(result, 0, strlen(content_to_write) + 1, third_read_buffer);
    assert(res_6 == strlen(content_to_write) + 1);
    assert(strcmp(third_read_buffer, "soorkie is the god!!") == 0);
    printf("[RAMFS TEST] Test passed\n");
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
    test_ramfs_write_and_read_normal();
    return 0;
}