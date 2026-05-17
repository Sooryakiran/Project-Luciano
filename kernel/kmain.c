#include <limine.h>
#include "debug.h"
#include "arch.h"
#include "boot.h"

static volatile LIMINE_BASE_REVISION(2);

void kmain(void) {

    k_init();
    k_log("Hello, World!");
    
    boot_info info;
    get_boot_entry(&info);

    arch_init();
    pmm_init(info.regions, info.region_count);
    // paddr_t f1 = pmm_alloc();
    // pmm_free(f1);
    // paddr_t f2 = pmm_alloc();
    // paddr_t f3 = pmm_alloc();
    // k_log("[PMM] frame1: %lu", f1);
    // k_log("[PMM] frame2: %lu", f2);
    // k_log("[PMM] frame3: %lu", f3);
    for(;;) {
    }
}
