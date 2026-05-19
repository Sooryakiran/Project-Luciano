#include <limine.h>
#include "debug.h"
#include "arch.h"
#include "boot.h"
#include "pmm.h"
#include "vmm.h"

static volatile LIMINE_BASE_REVISION(2);

void kmain(void) {

    k_init();
    k_log("Hello, World!");
    
    boot_info info;
    get_boot_entry(&info);

    arch_init();
    pmm_init(info.regions, info.region_count, info.hhdm_offset);
    vmm_init(info.kernel_physical_addr, info.kernel_virtual_addr, info.hhdm_offset);
    
    // paddr_t f1 = pmm_alloc();
    // pmm_free(f1);
    // paddr_t f2 = pmm_alloc();
    // paddr_t f3 = pmm_alloc();
    // k_log("[PMM] frame1: %b", f1);
    // k_log("[PMM] frame2: %b", f2);
    // k_log("[PMM] frame3: %b", f3);

    // k_log("[KERNEL] physical address %lu", info.kernel_physical_addr);
    // k_log("[KERNEL] virtual address %lu", info.kernel_virtual_addr);

    for(;;) {
    }
}
