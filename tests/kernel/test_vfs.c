#include <assert.h>
#include "stdio.h"
#include "vfs.h"
#include "string.h"

typedef struct vfs_test_case
{
    const char *path;
    const char *parsed_path[VFS_MAX_DEPTH];
    const uint8_t result;
} vfs_test_case_t;

void helper_test(vfs_test_case_t test_case)
{
    printf("VFS Path test case:\n");
    vfs_path_t result;
    uint8_t res = vfs_aux_parse_path(test_case.path, &result);
    assert(res == test_case.result);
    int j = 0;
    while (test_case.parsed_path[j] != NULL)
    {
        printf("%s vs %s\n", test_case.parsed_path[j], result.path[j]);
        assert(strcmp(test_case.parsed_path[j], result.path[j]) == 0);
        j++;
    }
    printf("Depth %d vs %d\n", j, result.depth);
    assert(j == result.depth);
    printf("VFS Path test case passed!\n");
}

void test_easy_path_parsing()
{
    printf("Testing easy path parsing...\n");
    vfs_test_case_t test_cases[] = {
        (vfs_test_case_t){
            .path = "home/soorkie/unemployed",
            .parsed_path = {"home", "soorkie", "unemployed"},
            .result = 0},
        (vfs_test_case_t){
            .path = "home/soorkie/unemployed/",
            .parsed_path = {"home", "soorkie", "unemployed"},
            .result = 0},
        (vfs_test_case_t){
            .path = "/home/soorkie/unemployed",
            .parsed_path = {"home", "soorkie", "unemployed"},
            .result = 0},
        (vfs_test_case_t){
            .path = "/home/soorkie/./unemployed",
            .parsed_path = {"home", "soorkie", "unemployed"},
            .result = 0},
        (vfs_test_case_t){
            .path = "/home/",
            .parsed_path = {"home"},
            .result = 0},
        (vfs_test_case_t){
            .path = "/home/soorkie/../unemployed",
            .parsed_path = {"home", "unemployed"},
            .result = 0},
        (vfs_test_case_t){
            .path = "/hello",
            .parsed_path = {"hello"},
            .result = 0},
        (vfs_test_case_t){
            .path = "/",
            .parsed_path = {},
            .result = 0},
        (vfs_test_case_t){
            .path = "/soorkie.txt",
            .parsed_path = {"soorkie.txt"},
            .result = 0},
        (vfs_test_case_t){
            .path = "soorkie.txt",
            .parsed_path = {"soorkie.txt"},
            .result = 0},
        (vfs_test_case_t){
            .path = "/./.soorkie.txt",
            .parsed_path = {".soorkie.txt"},
            .result = 0},
        (vfs_test_case_t){
            .path = "/./.soorkie.txt./",
            .parsed_path = {".soorkie.txt."},
            .result = 0}
        };

    for (int i = 0; i < sizeof(test_cases) / sizeof(vfs_test_case_t); i++)
    {
        helper_test(test_cases[i]);
    }
}

// void vfs_dentry_get_child_exists_test()
// {
//     printf("VFS Dentry existing child get\n");
//     vfs_dentry_t child_one = {
//         .name = "home",
//         .child_count = 0,
//         .children = {}};
//     vfs_dentry_t child_two = {
//         .name = "away",
//         .child_count = 0,
//         .children = {}};
//     vfs_dentry_t parent = {
//         .children = {&child_one, &child_two},
//         .child_count = 2,
//         .name = "root",
//         .parent = NULL,
//     };

//     vfs_dentry_t *result = NULL; // pointer to dentry
//     vfs_return_flag res = vfs_dentry_get_child(&parent, "home", &result);

//     assert(res == VFS_OK);
//     assert(result == &child_one);
//     printf("VFS Dentry test passed!\n");
// }

// void vfs_dentry_get_child_noexists_test()
// {
//     printf("VFS Dentry not existing child get\n");
//     vfs_dentry_t child_one = {
//         .name = "home",
//         .child_count = 0,
//         .children = {}};
//     vfs_dentry_t child_two = {
//         .name = "away",
//         .child_count = 0,
//         .children = {}};
//     vfs_dentry_t parent = {
//         .children = {&child_one, &child_two},
//         .child_count = 2,
//         .name = "root",
//         .parent = NULL,
//     };

//     vfs_dentry_t *result = NULL; // pointer to dentry
//     vfs_return_flag res = vfs_dentry_get_child(&parent, "doyir", &result);

//     assert(res == VFS_ENOENT);
//     printf("VFS Dentry no exist test passed!\n");
// }

// void vfs_dentry_put_child_test()
// {
//     printf("VFS Dentry put child test\n");
//     vfs_dentry_t child_one = {
//         .name = "home",
//         .child_count = 0,
//         .children = {}};
//     vfs_dentry_t child_two = {
//         .name = "away",
//         .child_count = 0,
//         .children = {}};

//     vfs_dentry_t parent = {
//         .children = {},
//         .child_count = 0,
//         .name = "root",
//         .parent = NULL,
//     };

//     vfs_return_flag res_0 = vfs_dentry_put_child(&parent, &child_one);
//     assert(res_0 == VFS_OK);
//     assert(parent.child_count == 1);
//     vfs_return_flag res_1 = vfs_dentry_put_child(&parent, &child_two);
//     assert(res_1 == VFS_OK);
//     assert(parent.child_count == 2);
//     vfs_return_flag res_2 = vfs_dentry_put_child(&parent, &child_one);
//     assert(res_2 == VFS_EEXIST);
//     assert(parent.child_count == 2);

//     vfs_dentry_t *result = NULL; // pointer to dentry
//     vfs_return_flag res = vfs_dentry_get_child(&parent, "home", &result);
//     assert(res == VFS_OK);

//     printf("VFS put child test passed!\n");
// }

int main()
{
    printf("VFS test suite..\n");
    // test_easy_path_parsing();
    // vfs_dentry_get_child_exists_test();
    // vfs_dentry_get_child_noexists_test();
    // vfs_dentry_put_child_test();
    return 0;
}