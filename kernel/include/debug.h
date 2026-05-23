#pragma once
#include "types.h"

void k_init(void);
void k_putc(char c);
void k_print(const char *s);

void k_log_hex(uint64_t);
void k_log(const char *s, ...);
void k_warn(const char *s, ...);
void k_error(const char *s, ...);
void k_panic(const char *s, ...);

#ifdef UNIT_TEST
int k_debug_get_num_panics();
void k_debug_unit_test_reset();
#endif