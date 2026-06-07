#include "kbuf.h"
#include "debug.h"

k_status kbuf_grow(kbuf_t *buf)
{
    size_t new_size = buf->allocated ? buf->allocated * 2 : 1;

    // overflow test
    if (new_size < buf->allocated)
        return K_STATUS_OVERFLOW;

#ifndef UNIT_TEST
    k_log("HELLO, size now is %d", new_size);
#endif

    char *new_data = (char *)kmalloc(sizeof(char) * new_size);

    // check if allocated properly
    if (!new_data)
        return K_STATUS_NOMEM;

    memcpy(new_data, buf->data, buf->allocated);
    kfree(buf->data);

    buf->data = new_data;
    buf->allocated = new_size;

    return K_STATUS_OK;
}

k_status kbuf_init(kbuf_t *buf, size_t new_size)
{
    if (new_size < buf->allocated)
        return K_STATUS_ERR;

    char *new_data = (char *)kmalloc(sizeof(char) * new_size);

    // check if allocated properly
    if (!new_data)
        return K_STATUS_NOMEM;

    kfree(buf->data);
    buf->data = new_data;
    buf->allocated = new_size;
    return K_STATUS_OK;
}