#include "string.h"
#include "types.h"
#include "debug.h"

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

int memcmp(const void *a, const void *b, size_t n)
{
    const unsigned char *astr = (char *)a;
    const unsigned char *bstr = (char *)b;

    for (int i = 0; i < n; i++)
    {
        k_log("[STRING] memcmp comparing %x and %x", astr[i], bstr[i]);
        if (astr[i] != bstr[i])
            return astr[i] - bstr[i];
    }
    return 0;
}

int strcmp(const char *a, const char *b)
{
    // k_log("[STRING] Comparing %s vs %s", a, b);
    while (*a && (*a == *b))
    {
        // k_log("CMP %c vs %c", *a, *b);
        a++;
        b++;
    };
    // k_log("CMP %d, %d", *(unsigned char *)a, *(unsigned char *)b);
    return *(unsigned char *)a - *(unsigned char *)b;
}

size_t strlen(const char *a)
{
    size_t len = 0;
    while (a[len] != '\0')
        len++;
    return len;
}