#pragma once

void k_init(void);
void k_putc(char c);
void k_print(const char *s);

void k_log(const char *s);
void k_warn(const char *s);
void k_error(const char *s);
void k_panic(const char *s);