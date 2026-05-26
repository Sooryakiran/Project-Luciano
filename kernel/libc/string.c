#include "string.h"
#include "types.h"

void *memset(void *ptr, int value, size_t size)
{
    uint8_t *p = (uint8_t *)ptr;
    for (size_t i = 0; i < size; i++)
    {
        p[i] = (uint8_t)value;
    }
    return ptr;
}

void *memcpy(void *dst, void *src, size_t size)
{
    uint8_t *pdst = (uint8_t *)dst;
    uint8_t *psrc = (uint8_t *)src;
    for (size_t i = 0; i < size; i++)
    {
        pdst[i] = psrc[i];
    }
    return dst;
}

int strcmp(const char *a, const char *b)
{
    while (*a & (*a == *b))
    {
        a++;
        b++;
    };
    return *(unsigned char *)a - *(unsigned char *)b;
}