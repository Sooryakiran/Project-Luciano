#include "kstring.h"
#include "types.h"
#include "string.h"
#include "debug.h"

char *kstrdup(const char *input)
{
    if (!input)
        return NULL;
    size_t len = strlen(input);
    char *out = (char *)kmalloc(len + 1);
    if (!out)
        return NULL;
    memcpy(out, input, len + 1);
    out[len] = '\0';
    k_log("[KSTRING] dupe is %s, original is %s", out, input);
    return out;
}