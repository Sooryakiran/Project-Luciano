#include <limine.h>
#include "debug.h"

static volatile LIMINE_BASE_REVISION(2);

void kmain(void) {
    debug_init();
    debug_print("Hello, World!\n");
    for(;;) {
    }
}