#include "kmalloc.h"
#include "pmm.h"
#include "vmm.h"
#include "debug.h"

#define K_HEAP_START 0xFFFFDEADBEEF0000 // deadbeef
#define K_HEAP_END 0xFFFFFFFF80000000 // kernel mapping starts here

static vaddr_t bump_pointer = K_HEAP_START;

void *kmalloc(size_t size) {
    if (size == 0) return NULL;

    size_t num_frames = (size + PMM_FRAME_SIZE - 1) / PMM_FRAME_SIZE;
    address_space_t space = vmm_get_current_space();

    vaddr_t result = bump_pointer;
    for (size_t i = 0; i < num_frames; i++) {

        // todo: handle failures
        paddr_t physical_addr = pmm_alloc();
        vaddr_t virtual_addr = bump_pointer;

        bump_pointer += PMM_FRAME_SIZE;
        vmm_map(space, virtual_addr, physical_addr, 0x3);
    }
    return (void *)result;
}

void kfree(void *ptr) {
    (void)ptr;
    // no op for now, we don't free physical or virtual
    // virtual is fine, physical? not so fine i guess
}

void kmalloc_init() {
    k_log("[KMALLOC] Kernel heap initialized to %x", K_HEAP_START);
}