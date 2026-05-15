#include <limine.h>
#include "debug.h"
#include "arch.h"

static volatile LIMINE_BASE_REVISION(2);

void kmain(void) {
    k_init();
    k_log("Hello, World!");
    arch_init();
    for(;;) {
    }
}