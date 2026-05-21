#include "drivers.h"
#include "boot.h"
#include "debug.h"
#include "drivers/framebuffer/fb.h"

void drivers_init(boot_info* info) {
    k_log("[DRV] Initializing drivers");
    fb_init(info->framebuffer, info->framebuffer_count);
}