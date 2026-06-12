#include "drivers/block_device/block_device.h"
#include "drivers/ide/ide.h"
#include "debug.h"
#include "kmalloc.h"

#define MAX_DRIVES 32

static uint16_t block_drives_count = 0;
static block_drive_t *block_drives[MAX_DRIVES];

void block_devices_init()
{
    k_log("[BLD] Initializing");
    ide_init();
    block_drive_t *ide_drives = ide_get_drives();
    uint16_t ide_counts = ide_get_drive_count();
    k_log("[BLD] Got %d IDE drives", ide_counts);
    block_devices_validate_and_add(
        ide_drives, ide_counts);

    k_log("[BLD] Done, discovered %d drives", block_drives_count);
}

void block_devices_validate_and_add(block_drive_t *drives, uint16_t counts)
{
    // uint16_t count = 0;
    for (uint16_t count = 0; count < counts; count++)
    {
        k_log("[BLD] Current count is %d", count);
        char *name = drives[count].block_drive_name;
        k_log("[BLD] Looking at %s", name);

        if (!drives[count].ops)
        {
            k_log("[BLD] Driver ops not initialized. Skipping!");
        }

        if (drives[count].ops->probe(&drives[count]) != BLD_SUCCESS)
        {
            k_log("[BLD] Failed to probe drive %s", name);
            continue;
        }
        k_log("[BLD] Probe success for %s", name);

        bld_size sector_size = drives[count].ops->get_sector_count(&drives[count]);
        if (sector_size == BLD_SIZE_ERR)
        {
            k_log("[BLD] Invalid sectors reported! Skipping device %s!", name);
            continue;
        }
        k_log("[BLD] Found %d sectors for block drive %s", sector_size, name);
        block_drives[block_drives_count++] = &drives[count];
    }
}