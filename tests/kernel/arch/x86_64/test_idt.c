#include <assert.h>
#include <stdio.h>
#include "types.h"
#include "arch/x86_64/idt.h"

void test_create_idt_entry_offset() {
    printf("Testing create idt offsets....\n");
    paddr_t offsets[] = {
        0, 1, 0xDEAFBEEF, 0xF8BADF00D, 0xCAFEBABE
    };
    for (int i = 0; i < sizeof(offsets) / sizeof(paddr_t); i++) {
        void* offset = (void *)offsets[i];
        idt_entry entry = create_idt_entry(offset, 0);
        uint64_t offset_res = 0;
        offset_res |= (uint64_t)entry.offset_low;
        offset_res |= ((uint64_t)entry.offset_mid << 16);
        offset_res |= ((uint64_t)entry.offset_high << 32);
        assert((uint64_t)offset_res == (uint64_t)offset);
    }
    printf("Test Passed");
}

int main() {
    test_create_idt_entry_offset();
}