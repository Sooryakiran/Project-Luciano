#include "debug.h"
#include "io.h"

void k_init() {
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x80);
    outb(0x3F8 + 0, 0x03);
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x03);
    outb(0x3F8 + 2, 0xC7);
}

void k_putc(char c) {
    while (!(inb(0x3F8 + 5) & 0x20));
    outb(0x3F8, c);
}

void k_print(const char *s) {
    while (*s) k_putc(*s++);
}

void k_print_format(const char *t, const char *s) {
    k_print(t);
    k_print(s);
    k_print("\n");
}

void k_log(const char *s) {
    k_print_format("[LOG] ", s);
}

void k_warn(const char *s) {
    k_print_format("[WARN] ", s);
}

void k_error(const char *s) {
    k_print_format("[ERROR] ", s);
}

void k_panic(const char *s) {
    k_print_format("[PANIC] ", s);
    for(;;); // halt
}
