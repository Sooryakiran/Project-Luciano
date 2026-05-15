#include <assert.h>
#include <stdio.h>
#include "arch/x86_64/gdt.h"
#include "types.h"
#include "debug.h"

void test_null_descriptor() {
    printf("Testing null descriptor...\n");
    gdt_entry_t result = create_gdt_entry((gdt){0, 0, 0, 0});
    assert(result == 0);
    printf("Null descriptor test passed!\n");
}

void test_flags_valid_flags() {
    printf("Testing valid flags...\n");
    uint8_t valid_flags[] = {
        0, 3, 5, 8, 15
    };
    for (int i = 0; i < sizeof(valid_flags) / sizeof(uint8_t); i++) {
        gdt_entry_t result = create_gdt_entry((gdt){0, 0, 0, valid_flags[i]});
        uint8_t flag = (uint8_t)(result >> 52) & 0xF;
        assert(flag == valid_flags[i]);
    }
    printf("Test valid flags passed\n");
}

void test_flags_invalid_flags() {
    printf("Testing invalid flags...\n");
    uint8_t invalid_flags[] = {
        16, 20, 21
    };

    for (int i = 0; i < sizeof(invalid_flags) / sizeof(uint8_t); i++) {
        k_debug_unit_test_reset();
        gdt_entry_t result = create_gdt_entry((gdt){0, 0, 0, invalid_flags[i]});
        assert(k_debug_get_num_panics() == 1);
    }
    printf("Test invalid flags passed\n");
}

void test_access_byte() {
    printf("Testing access byte...\n");
    uint8_t access_bytes[] = {
        1, 6, 18, 24, 31
    };

    for (int i = 0; i < sizeof(access_bytes) / sizeof(uint8_t); i++) {
        gdt_entry_t result = create_gdt_entry((gdt){0, 0, 0, access_bytes[i]});
        uint8_t access_byte = (uint8_t)(result >> 40);
        assert(access_byte = access_bytes[i]);
    } 
    printf("Test passed\n");
}

void test_valid_base() {
    printf("Testing valid base...\n");

    uint32_t valid_bases[] = {
        1024, 4096, 2147483648
    };

    for (int i = 0; i < sizeof(valid_bases) / sizeof(uint32_t); i++) {
        gdt_entry_t result = create_gdt_entry((gdt){valid_bases[i], 0, 0, 0});
        uint32_t valid_base = 0;
        valid_base |= ((uint8_t)(result >> 56) << 24);
        valid_base |= (uint32_t)(result >> 16) & (0xFFFFFF);
        assert(valid_base == valid_bases[i]);
    }

    printf("Test passed.\n");
}

void test_valid_limit() {
    printf("Testing valid limits\n");
    uint32_t valid_limits[] = {
        1024, 4096, 123456
    };

    for (int i = 0; i < sizeof(valid_limits) / sizeof(uint32_t); i++) {
        gdt_entry_t result = create_gdt_entry((gdt){0, valid_limits[i], 0, 0});
        uint32_t limit = 0;
        limit |= ((uint8_t)(result >> 48) << 16);
        limit |= (uint16_t)(result);
        assert(limit = valid_limits[i]);
    }
    printf("Test passed!\n");
}

void test_invalid_limit() {
    printf("Testing invalid limits");
    uint32_t invalid_limits[] = {
        0xFFFFF + 1, 0xFFFFF + 1234, 2147483648
    };

    for (int i = 0; i < sizeof(invalid_limits) / sizeof(uint32_t); i++) {
        k_debug_unit_test_reset();
        gdt_entry_t result = create_gdt_entry((gdt){0, invalid_limits[i], 0, 0});
        assert(k_debug_get_num_panics() == 1);
    }
    printf("Test passed");
}

int main() {
    test_null_descriptor();
    test_flags_valid_flags();
    test_flags_invalid_flags();
    test_access_byte();
    test_valid_base();
    test_valid_limit();
    test_invalid_limit();
    return 0;
}