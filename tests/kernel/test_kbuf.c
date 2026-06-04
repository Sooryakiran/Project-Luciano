#include <assert.h>
#include "stdio.h"
#include "kbuf.h"

kbuf_t create_buffer(size_t size)
{
    return (kbuf_t){
        .data = kmalloc(sizeof(char) * size),
        .allocated = size};
}

void kbuf_grow_allocation_happy_test()
{
    printf("[KBUF_TEST] Happy growth test\n");
    kbuf_t test_cases[] = {
        create_buffer(1),
        create_buffer(1024),
        create_buffer(14),
    };

    for (int i = 0; i < sizeof(test_cases) / sizeof(kbuf_t); i++) {
        uint64_t prev_size = test_cases[i].allocated;
        kbuf_grow(&(test_cases[i]));
        assert(test_cases[i].allocated == prev_size * 2);
    }
    printf("[KBUF_TEST] Test passed!\n");
}

void kbuf_grow_memory_gets_copied_test() 
{
    printf("[KBUF_TEST] Memory copy growth test\n");
    char *simple_test = kmalloc(8);
    char *simple_result = kmalloc(8);
    memcpy(simple_test, "soorkie", 8);
    memcpy(simple_result, "soorkie", 8);

    uint64_t initial_size = sizeof(simple_test);
    kbuf_t test_case = {
        .data = simple_test,
        .allocated = sizeof(simple_test)
    };
    kbuf_grow(&test_case);

    assert(strcmp(simple_result, test_case.data) == 0);

    printf("[KBUF_TEST] Test passed!\n");
}

int main()
{
    printf("[KBUF_TEST] Kbuf test suite...\n");
    kbuf_grow_allocation_happy_test();
    kbuf_grow_memory_gets_copied_test();
    return 0;
}