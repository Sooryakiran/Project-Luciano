#pragma once

#ifndef UNIT_TEST
#define NULL ((void *)0)
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;
typedef uint64_t size_t;  // we are 64 bit
#else
#include <stdint.h>
#include <stddef.h>
#endif

typedef long long int int64;

typedef uint64_t paddr_t; // physical address
typedef uint64_t vaddr_t; // virtual address alias

typedef enum {
    PRIVILEGE_KERNEL = 0,
    PRIVILEGE_USER   = 1,
} privilege_t;

typedef enum {
    K_STATUS_OK,
    K_STATUS_OVERFLOW,
    K_STATUS_ERR,
    K_STATUS_NOMEM
} k_status;