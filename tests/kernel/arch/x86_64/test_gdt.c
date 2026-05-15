#include <assert.h>
#include <stdio.h>
#include "arch/x86_64/gdt.h"

void test_null_descriptor() {
    printf("Testing null descriptor...\n");
    gdt_entry_t result = create_gdt_entry((gdt){0, 0, 0, 0});
    assert(result == 0);
    printf("Null descriptor test passed!\n");
}

int main() {
    test_null_descriptor();
    return 0;
}