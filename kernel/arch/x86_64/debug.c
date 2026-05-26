#include "debug.h"
#include "arch/x86_64/io.h"
#include <stdarg.h>

#ifndef UNIT_TEST
void k_init()
{
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x80);
    outb(0x3F8 + 0, 0x03);
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x03);
    outb(0x3F8 + 2, 0xC7);
}

void k_putc(char c)
{
    while (!(inb(0x3F8 + 5) & 0x20))
        ;
    outb(0x3F8, c);
}

void k_print(const char *s)
{
    while (*s)
        k_putc(*s++);
}

void k_printf_lu(uint64_t n)
{
    char buf[32];
    int i = 0;
    while (n > 0)
    {
        buf[i++] = '0' + n % 10;
        n /= 10;
    }
    while (i--)
        k_putc(buf[i]);
}

void k_printf_d(int n)
{
    char buf[32];
    int i = 0;
    if (n == 0)
    {
        k_putc('0');
        return;
    }
    if (n < 0)
    {
        k_putc('-');
        n = -n;
    }
    while (n > 0)
    {
        buf[i++] = '0' + (n % 10);
        n /= 10;
    }

    while (i--)
    {
        k_putc(buf[i]);
    }
}

void k_printf_b(uint64_t n)
{
    char buf[64];
    if (n == 0)
    {
        k_putc('0');
        return;
    }
    int i = 0;
    while (n > 0)
    {
        buf[i++] = ((n & 0x1) == 1) ? '1' : '0';
        n >>= 1;
    }
    while (i--)
    {
        k_putc(buf[i]);
    }
}

void k_printf_x(uint64_t n)
{

    char buf[64];
    const char *char_map = "0123456789abcdef";
    // char char_map[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    if (n == 0)
    {
        k_putc('0');
        return;
    }
    int i = 0;
    while (n > 0)
    {
        buf[i++] = char_map[(n % 16)];
        n >>= 4;
    }
    while (i--)
    {
        k_putc(buf[i]);
    }
}

void k_printf(const char *s, va_list args)
{
    while (*s)
        if (*s == '%')
        {
            s++;
            switch (*s)
            {
            case 'l':
            {
                s++;
                if (*s == 'u')
                {
                    uint64_t n = va_arg(args, uint64_t);
                    k_printf_lu(n);
                    s++;
                }
                break;
            }
            case 'd':
            {
                int n;
                n = va_arg(args, int);
                k_printf_d(n);
                s++;
                break;
            }

            case 'b':
            {
                uint64_t n;
                n = va_arg(args, uint64_t);
                k_printf_b(n);
                s++;
                break;
            }

            case 'x':
            {
                uint64_t n;
                n = va_arg(args, uint64_t);
                k_printf_x(n);
                s++;
                break;
            }

            default:
            {
                k_putc('%');
                break;
            }
            }
        }
        else
        {
            k_putc(*s++);
        }
}

void k_print_format(const char *t, const char *s, va_list args)
{
    k_print(t);
    k_printf(s, args);
    k_print("\n");
}

void k_log_hex(uint64_t value)
{
    k_log("Value %x", value);
}

void k_log(const char *s, ...)
{
    va_list args;
    va_start(args, s);
    k_print_format("[LOG] ", s, args);
    va_end(args);
}

void k_warn(const char *s, ...)
{
    va_list args;
    va_start(args, s);
    k_print_format("[WARN] ", s, args);
    va_end(args);
}

void k_error(const char *s, ...)
{
    va_list args;
    va_start(args, s);
    k_print_format("[ERROR] ", s, args);
    va_end(args);
}

void k_panic(const char *s, ...)
{
    va_list args;
    va_start(args, s);
    k_print_format("[PANIC] ", s, args);
    va_end(args);

    for (;;)
        ; // halt
}

#else

#include <stdio.h>
int k_debug_panic_called = 0;

void k_debug_unit_test_reset()
{
    k_debug_panic_called = 0;
}

int k_debug_get_num_panics()
{
    return k_debug_panic_called;
}

void k_init(void) {}
void k_putc(char c) {}
void k_print(const char *s) {}
void k_log_hex(uint64_t n) {}
void k_log(const char *s, ...)
{
    va_list args;
    va_start(args, s);
    vprintf(s, args);
    va_end(args);
}
void k_warn(const char *s, ...) {}
void k_error(const char *s, ...) {}
void k_panic(const char *s, ...)
{
    k_debug_panic_called += 1;
}
#endif