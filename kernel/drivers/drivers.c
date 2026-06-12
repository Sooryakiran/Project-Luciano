#include "drivers.h"
#include "boot.h"
#include "debug.h"
#include "drivers/framebuffer/fb.h"
#include "drivers/block_device/block_device.h"

void drivers_init(boot_info* info) {
    k_log("[DRV] Initializing drivers");
    fb_init(info->framebuffer, info->framebuffer_count);
    block_devices_init();
}