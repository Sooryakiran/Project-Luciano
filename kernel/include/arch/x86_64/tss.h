#pragma once
#include "types.h"

typedef struct tss {
    uint32_t reserved0;
    vaddr_t rsp0;
    vaddr_t rsp1;
    vaddr_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint32_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} __attribute__((packed)) tss_t;


void tss_init();
void tss_update(vaddr_t rsp0);
tss_t *tss_get();