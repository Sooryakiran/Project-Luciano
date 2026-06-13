#include <assert.h>
#include "stdio.h"
#include "drivers/ramfs/ramfs.h"
#include "string.h"
#include "vfs.h"

void test_ramfs_mount()
{
    printf("[RAMFS TEST] ramfs mount test\n");
    vfs_init();
    ramfs_init();
    vfs_return_flag res_0 = vfs_mount("/", ramfs_get_sb());
    assert(res_0 == VFS_OK);
    vfs_return_flag res_1 = vfs_mount("/", ramfs_get_sb());
    assert(res_1 != VFS_OK);
    printf("[RAMFS TEST] test passed\n");
}

void test_setup()
{
    vfs_init();
    ramfs_init();
    vfs_return_flag res_0 = vfs_mount("/", ramfs_get_sb());
    assert(res_0 == VFS_OK);
}

void test_ramfs_open_read_noexists()
{
    test_setup();
    printf("[RAMFS TEST] Testing ramfs open not exists\n");
    vfs_file_descriptor_t *result = NULL;
    vfs_return_flag res = vfs_open("/home/soorkie", VFS_O_RDONLY, &result);
    assert(res == VFS_ENOENT);
    printf("[RAMFS TEST] Test passed!\n");
}

void test_ramfs_open_write_noexists() {
    test_setup();
    printf("[RAMFS TEST] Testing ramfs write not exists\n");
    vfs_file_descriptor_t* result = NULL;
    vfs_return_flag res = vfs_open("/home/soorkie", VFS_O_WRONLY, &result);
    assert(res == VFS_ENOENT);
    printf("[RAMFS TEST] Test passed!\n");
}

void test_ramfs_open_write_with_create() {
    test_setup();
    printf("[RAMFS TEST] Testing ramfs write with create not exists\n");
    vfs_file_descriptor_t* result = NULL;
    vfs_return_flag res = vfs_open("/soorkie", VFS_O_WRONLY | VFS_O_CREAT, &result);
    assert(res == VFS_OK);
    assert(result->flags == (VFS_O_WRONLY | VFS_O_CREAT));
    assert(result->inode->size == 0);
    assert(strcmp(result->dentry->name, "soorkie") == 0);
    printf("[RAMFS TEST] Test passed\n");
}

void test_ramfs_open_read_existing() {
    test_setup();
    printf("[RAMFS TEST] Testing ramfs read existing\n");
    vfs_file_descriptor_t* result = NULL;
    vfs_return_flag res = vfs_open("/soorkie", VFS_O_WRONLY | VFS_O_CREAT, &result);
    assert(res == VFS_OK);
    vfs_return_flag res_2 = vfs_open("/soorkie", VFS_O_RDONLY, &result);
    assert(strcmp(result->dentry->name, "soorkie") == 0);
    assert(res_2 == VFS_OK);
}

void test_ramfs_open_write_with_create_nested() {
    printf("[RAMFS TEST] Testing ramfs write with create not exists but nested\n");
    test_setup();
    vfs_file_descriptor_t* result = NULL;
    vfs_return_flag res = vfs_open("/nested/soorkie", VFS_O_WRONLY | VFS_O_CREAT, &result);
    assert(res == VFS_ENOENT);
    printf("[RAMFS TEST] Test passed\n");
}

void test_ramfs_open_write_existing_with_excl() {
    test_setup();
    printf("[RAMFS TEST] Testing ramfs read existing\n");
    vfs_file_descriptor_t* result = NULL;
    vfs_return_flag res = vfs_open("/soorkie_exists", VFS_O_WRONLY | VFS_O_CREAT, &result);
    assert(res == VFS_OK);
    vfs_return_flag res_2 = vfs_open("/soorkie_exists", VFS_O_WRONLY | VFS_O_CREAT | VFS_O_EXCL, &result);
    assert(res_2 == VFS_EEXIST);
}

void test_ramfs_mkdir() {
    test_setup();
    printf("[RAMFS TEST] Testing ramfs mkdir\n");

    vfs_return_flag res_0 = vfs_mkdir("/home");
    assert(res_0 == VFS_OK);

    vfs_return_flag res_1 = vfs_mkdir("/home");
    assert(res_1 == VFS_EEXIST);

    vfs_return_flag res_2 = vfs_mkdir("/soorkie/images");
    assert(res_2 == VFS_ENOENT);

    vfs_return_flag res_3 = vfs_mkdir("soorkie");
    assert(res_3 == VFS_OK);

    vfs_return_flag res_4 = vfs_mkdir("/soorkie/images");
    assert(res_4 == VFS_OK);

    printf("[RAMFS TEST] Test passed \n");
}

void test_ramfs_write_and_read_normal() {
    test_setup();
    printf("[RAMFS TEST] Testing ramfs write and read happy path.\n");

    // open a file
    vfs_file_descriptor_t* result = NULL;
    vfs_return_flag res = vfs_open("/soorkie", VFS_O_WRONLY | VFS_O_CREAT, &result);
    assert(res == VFS_OK);
    printf("[RAMFS TEST] Opened file\n");

    // write to that place
    char *content_to_write = "soorkie is the king!";
    vfs_size res_1 = vfs_write(result, 0, strlen(content_to_write) + 1, content_to_write);
    assert(res_1 == strlen(content_to_write) + 1);

    // now read
    char *read_buffer = kmalloc(strlen(content_to_write) + 1);
    vfs_size res_2 = vfs_read(result, 0, strlen(content_to_write) + 1, read_buffer);
    assert(strcmp(content_to_write, read_buffer) == 0);
    assert(res_2 == strlen(content_to_write) + 1);

    // now write to some other place
    char *second_content_to_write = "hahahaha, i am the ruler of earth";
    vfs_size res_3 = vfs_write(result, 1024, strlen(second_content_to_write) + 1, second_content_to_write);
    assert(res_3 == strlen(second_content_to_write) + 1);

    // now read that part
    char *second_read_buffer = kmalloc(strlen(second_content_to_write) + 1);
    vfs_size res_4 = vfs_read(result, 1024, strlen(second_content_to_write) + 1, second_read_buffer);
    assert(res_4 == strlen(second_content_to_write) + 1);
    assert(strcmp(second_content_to_write, second_read_buffer) == 0);

    // now replace king with "god!"
    char *replacer = "god!";
    vfs_size res_5 = vfs_write(result, 15, 4, replacer);
    assert(res_5 = 4);
    char *third_read_buffer = kmalloc(strlen(content_to_write) + 1);
    vfs_size res_6 = vfs_read(result, 0, strlen(content_to_write) + 1, third_read_buffer);
    assert(res_6 == strlen(content_to_write) + 1);
    assert(strcmp(third_read_buffer, "soorkie is the god!!") == 0);
    printf("[RAMFS TEST] Test passed\n");
}

void test_ramfs_open_adirectory() {
    test_setup();
    printf("[RAMFS TEST] Testing ramfs opening directory\n");

    vfs_return_flag res_0 = vfs_mkdir("/hello");
    assert(res_0 == VFS_OK);

    vfs_file_descriptor_t* fd = NULL;
    vfs_return_flag res_1 = vfs_open("/hello", VFS_O_RDONLY, &fd);
    assert(res_1 == VFS_OK);
    assert(fd->inode->mode == VFS_INODE_DIR);
    printf("[RAMFS TEST] Directory opening test passed!\n");
}

void test_ramfs_open_root() {
    test_setup();
    printf("[RAMFS TEST] Testing opening root\n");
    vfs_file_descriptor_t* fd = NULL;
    vfs_return_flag res = vfs_open("/", VFS_O_RDONLY, &fd);
    assert(res == VFS_OK);
    assert(fd->inode->mode = VFS_INODE_DIR);
    printf("[RAMFS TEST] Test passed\n");
}

void test_ramfs_emit_no_size() {
    printf("[RAMFS TEST] Testing ramfs emit dentry no size\n");
    vfs_inode_t inode = {
        .ino = 42,
        .mode = VFS_INODE_REG
    };

    size_t alloc_size = 3;
    void *buff = kmalloc(alloc_size);
    uint16_t res = ramfs_emit_entry(buff, alloc_size, &inode, "shawarma");
    assert(res == 0);
    printf("[RAMFS TEST] Test passed!\n");
}

void test_ramfs_emit_happy() {
    printf("[RAMFS TEST] Testing ramfs emit happy\n");
    vfs_inode_t inode = {
        .ino = 37,
        .mode = VFS_INODE_REG
    };

    size_t alloc_size = 1024;
    void *buff = kmalloc(alloc_size);
    u_int16_t res = ramfs_emit_entry(buff, alloc_size, &inode, "shawarma");
    assert(res != 0);

    vfs_dir_entry_t *entry = (vfs_dir_entry_t *)buff;
    assert(entry->ino == inode.ino);
    assert(entry->type == inode.mode);
    assert(entry->name_len == strlen("shawarma"));
    assert(entry->rec_len = res);
    assert(strcmp(entry->name, "shawarma") == 0);
    printf("[RAMFS TEST] Test passed\n");
}

typedef struct {
    const char *name;
    vfs_mode_t type;
} ramfs_dir_entry_testcase;

void aux_helper_readdir_verify(void *buffer, size_t buffer_len, ramfs_dir_entry_testcase* test_cases, size_t test_case_count) {
    uint64_t pos = 0;
    for(int i = 0; i < test_case_count; i++) {

        assert(pos < buffer_len);
        vfs_dir_entry_t *entry = (vfs_dir_entry_t *)(buffer + pos);

        ramfs_dir_entry_testcase test_case = test_cases[i];
        printf("[RAMFS TEST]\t verifying %s vs %s\n", entry->name, test_cases[i].name);
        assert(strcmp(entry->name, test_cases[i].name) == 0);
        assert(entry->type == test_cases[i].type);
        pos += entry->rec_len;
    }
    assert(pos >= buffer_len);
}

void test_ramfs_readdir() {
    test_setup();
    printf("[RAMFS TEST] Testing read directory\n");

    // ls /
    vfs_file_descriptor_t *rootfp = NULL;
    vfs_return_flag res_0 = vfs_open("/", VFS_O_RDONLY, &rootfp);
    assert(res_0 == 0);
    printf("[RAMFS TEST] \t opened root\n");
    void *read_home_buffer = kmalloc(1024);
    vfs_size res_1 = vfs_readdir(rootfp, 0, 1024, read_home_buffer);
    printf("[RAMFS TEST] \t read %d entries\n", res_1);

    // read entries, look for "." and ".."
    ramfs_dir_entry_testcase test_cases_0[] = {
        (ramfs_dir_entry_testcase){
            .name = ".",
            .type = VFS_INODE_DIR
        },
        (ramfs_dir_entry_testcase) {
            .name = "..",
            .type = VFS_INODE_DIR
        }
    };
    aux_helper_readdir_verify(read_home_buffer, res_1, test_cases_0, 2);

    // touch /soorkie.exe
    vfs_file_descriptor_t *new_file = NULL;
    vfs_return_flag res_2 = vfs_open("/soorkie.txt", VFS_O_CREAT | VFS_O_WRONLY, &new_file);
    assert(res_2 == VFS_OK);

    // ls /
    vfs_size res_3 = vfs_readdir(rootfp, 0, 1024, read_home_buffer);

    // read entries, look for ".", ".." and "soorkie.txt"
    ramfs_dir_entry_testcase test_cases_1[] = {
        (ramfs_dir_entry_testcase){
            .name = ".",
            .type = VFS_INODE_DIR
        },
        (ramfs_dir_entry_testcase) {
            .name = "..",
            .type = VFS_INODE_DIR
        },
        (ramfs_dir_entry_testcase) {
            .name = "soorkie.txt",
            .type = VFS_INODE_REG
        }
    };
    aux_helper_readdir_verify(read_home_buffer, res_3, test_cases_1, 3);

    // mkdir /nested
    vfs_return_flag res_4 = vfs_mkdir("/nested");
    assert(res_4 == VFS_OK);

    ramfs_dir_entry_testcase test_cases_2[] = {
        (ramfs_dir_entry_testcase){
            .name = ".",
            .type = VFS_INODE_DIR
        },
        (ramfs_dir_entry_testcase) {
            .name = "..",
            .type = VFS_INODE_DIR
        },
        (ramfs_dir_entry_testcase) {
            .name = "soorkie.txt",
            .type = VFS_INODE_REG
        },
        (ramfs_dir_entry_testcase) {
            .name = "nested",
            .type = VFS_INODE_DIR
        }
    };
    vfs_size res_5 = vfs_readdir(rootfp, 0, 1024, read_home_buffer);
    aux_helper_readdir_verify(read_home_buffer, res_5, test_cases_2, 4);

    // touch /nested/virus.exe
    vfs_file_descriptor_t *virus_file = NULL;
    vfs_return_flag res_6 = vfs_open("/nested/virus.exe", VFS_O_WRONLY | VFS_O_CREAT, &virus_file);
    assert(res_6 == VFS_OK);

    // cd nested
    vfs_file_descriptor_t *nested_dir = NULL;
    vfs_return_flag res_7 = vfs_open("nested/./", VFS_O_RDONLY, &nested_dir);
    assert(res_7 == VFS_OK);

    // ls
    vfs_size res_8 = vfs_readdir(nested_dir, 0, 1024, read_home_buffer);
    ramfs_dir_entry_testcase test_cases_3[] = {
        (ramfs_dir_entry_testcase){
            .name = ".",
            .type = VFS_INODE_DIR
        },
        (ramfs_dir_entry_testcase) {
            .name = "..",
            .type = VFS_INODE_DIR
        },
        (ramfs_dir_entry_testcase) {
            .name = "virus.exe",
            .type = VFS_INODE_REG
        }
    };
    aux_helper_readdir_verify(read_home_buffer, res_8, test_cases_3, 3);
    printf("[RAMFS TEST] Test passed!\n");
}

void test_ramfs_readdir_offset() {
    test_setup();
    printf("[RAMFS TEST] Testing read directory offset\n");

    // ls /
    vfs_file_descriptor_t *rootfp = NULL;
    vfs_return_flag res_0 = vfs_open("/", VFS_O_RDONLY, &rootfp);

    void *read_home_buffer = kmalloc(1024);

    // touch /soorkie.exe
    vfs_file_descriptor_t *new_file = NULL;
    vfs_return_flag res_2 = vfs_open("/soorkie.txt", VFS_O_CREAT | VFS_O_WRONLY, &new_file);
    assert(res_2 == VFS_OK);

    // ls /
    vfs_size res_3 = vfs_readdir(rootfp, 1, 1024, read_home_buffer);

    // read entries, look for ".", ".." and "soorkie.txt"
    ramfs_dir_entry_testcase test_cases_1[] = {
        (ramfs_dir_entry_testcase) {
            .name = "..",
            .type = VFS_INODE_DIR
        },
        (ramfs_dir_entry_testcase) {
            .name = "soorkie.txt",
            .type = VFS_INODE_REG
        }
    };
    aux_helper_readdir_verify(read_home_buffer, res_3, test_cases_1, 2);
    printf("[RAMFS TEST] Test passed!\n");
}

void test_ramfs_readdir_nospace() {
    test_setup();
    printf("[RAMFS TEST] Testing read directory no space\n");

    // ls /
    vfs_file_descriptor_t *rootfp = NULL;
    vfs_return_flag res_0 = vfs_open("/", VFS_O_RDONLY, &rootfp);

    void *read_home_buffer = kmalloc(72);

    // touch /soorkie.exe
    vfs_file_descriptor_t *new_file = NULL;
    vfs_return_flag res_2 = vfs_open("/soorkie.txt", VFS_O_CREAT | VFS_O_WRONLY, &new_file);
    assert(res_2 == VFS_OK);

    // ls /
    vfs_size res_3 = vfs_readdir(rootfp, 0, 72, read_home_buffer);
    // read entries, look for ".", ".." and "soorkie.txt"
    ramfs_dir_entry_testcase test_cases_1[] = {
        (ramfs_dir_entry_testcase) {
            .name = ".",
            .type = VFS_INODE_DIR
        },
        (ramfs_dir_entry_testcase) {
            .name = "..",
            .type = VFS_INODE_DIR
        },
        (ramfs_dir_entry_testcase) {
            .name = "soorkie.txt",
            .type = VFS_INODE_REG
        }
    };
    aux_helper_readdir_verify(read_home_buffer, res_3, test_cases_1, 2);
    printf("[RAMFS TEST] Test passed!\n");
}

int main()
{
    printf("[RAMFS TEST] ramfs test suite...\n");
    test_ramfs_mount();
    test_ramfs_open_read_noexists();
    test_ramfs_open_write_noexists();
    test_ramfs_open_write_with_create();
    test_ramfs_open_write_with_create_nested();
    test_ramfs_open_read_existing();
    test_ramfs_open_write_existing_with_excl();
    test_ramfs_mkdir();
    test_ramfs_write_and_read_normal();
    test_ramfs_open_adirectory();
    test_ramfs_open_root();
    test_ramfs_emit_no_size();
    test_ramfs_emit_happy();
    test_ramfs_readdir();
    test_ramfs_readdir_offset();
    test_ramfs_readdir_nospace();
    return 0;
}