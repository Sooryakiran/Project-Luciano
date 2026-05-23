#include "arch/x86_64/tss.h"
#include "string.h"
#include "debug.h"

static tss_t tss;

void tss_init() {
    k_log("[TSS] Initializing TSS");
    memset(&tss, 0, sizeof(tss_t));
    tss.iomap_base = sizeof(tss_t);
    k_log("[TSS] Done");
}

void tss_update(vaddr_t rsp0) {
    tss.rsp0 = rsp0;
}

tss_t *tss_get() {
    return &tss;
}