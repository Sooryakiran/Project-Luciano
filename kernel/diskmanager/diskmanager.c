#include "diskmanager.h"
#include "block_device.h"
#include "debug.h"
#include "kmalloc.h"
#include "string.h"

static char magic_sign[2] = {0x55, 0xAA};

static disk_manager_volume_t *diskmanager_volumes[DSK_MAX_VOLUMES];
static uint16_t diskmanager_volume_counts = 0;

void diskmanager_log_volumes()
{
    k_log("[DSK] Found %d volumes.", diskmanager_volume_counts);
    for (uint16_t i = 0; i < diskmanager_volume_counts; i++)
    {
        k_log(
            "[DSK] Vol%d: %s lba[0x%x-0x%x)",
            i,
            diskmanager_volumes[i]->parent_disk->block_drive_name,
            diskmanager_volumes[i]->lba_start,
            diskmanager_volumes[i]->lba_end);
        
        // if (!diskmanager_volumes[i]->parent_disk->ops) {
        //     k_log("[DSK] Ops not found!");
        // }
    }
}

void diskmanager_init()
{
    k_log("[DSK] Initializing diskmanager");

    block_drive_t **block_drives = block_devices_get();
    uint16_t block_drives_count = block_devices_get_count();

    for (uint16_t i = 0; i < block_drives_count; i++)
    {
        diskmanager_setup_drive(block_drives[i]);
    }

    diskmanager_log_volumes();

    for (uint16_t i = 0; i < diskmanager_volume_counts; i++)
    {
        diskmanager_setup_partitions(diskmanager_volumes[i]);
    }
}

void diskmanager_setup_drive(block_drive_t *drive)
{
    const char *name = drive->block_drive_name;
    k_log("[DSK] Setting up drive %s", name);

    char *first_sector = (char *)kmalloc(drive->logical_sector_size);
    int read_count = drive->ops->read(drive, 0, 1, first_sector);
    if (read_count != 1)
    {
        k_log("[DSK] error reading %s", name);
        return;
    }

    if (memcmp(&first_sector[512 - 2], magic_sign, 2) != 0)
    {
        k_log("[DSK] No valid partition or format for disk %s, skipping!", name);
        return;
    }

    // check for MBR signature
    disk_manager_volume_t *mbr_out = NULL;
    int16 mbr_counts = diskmanager_mbr_check(first_sector, &mbr_out);
    if (mbr_counts > 0)
    {
        k_log("[DSK] I found %d MBR partitions for disk %s", mbr_counts, name);
        for (int i = 0; i < mbr_counts; i++)
        {
            mbr_out[i].parent_disk = drive;
            mbr_out[i].id = diskmanager_volume_counts;
            k_log("[DSK] Magic number for paritionis %d is 0x%x", mbr_out[i].id, mbr_out[i].magic_number);
            diskmanager_volumes[diskmanager_volume_counts++] = &(mbr_out[i]);
        }
    }
    else
    {
        k_log("[DSK] Possibly superfloppy");
        disk_manager_volume_t *superfloppy = kmalloc(sizeof(disk_manager_volume_t));
        superfloppy->lba_start = 0;
        superfloppy->lba_end = drive->ops->get_sector_count(drive);
        superfloppy->parent_disk = drive;
        superfloppy->id = diskmanager_volume_counts;
        diskmanager_volumes[diskmanager_volume_counts++] = superfloppy;
    }
    // first_sector[]
}

void diskmanager_setup_partitions(disk_manager_volume_t *vol)
{
    k_log("[DSK] Checking for partitions! %s", vol->parent_disk->block_drive_name);
    char *buffer = (char *)kmalloc(512);
    int res = diskmanager_read_volume(vol, 0, 1, buffer);
    k_log("[DSK] Read %d", res);
    for (int i = 0; i < 512; i++)
    {
        // k_log("EYE %d is %c", i, buffer[i]);
    }
}

int diskmanager_read_volume(disk_manager_volume_t *vol, uint64_t lba, uint32_t count, void *buffer)
{

    if (!vol)
    {
        k_log("[DSK] Volume not initialized!");
        return BLD_SIZE_ERR;
    }

    if (!vol->parent_disk)
    {
        k_log("[DSK] Parent disk not initialized!");
        return BLD_SIZE_ERR;
    }

    if (!vol->parent_disk->ops)
    {
        k_log("[DSK] Parent disk ops not initialized");
        return BLD_SIZE_ERR;
    }

    if (!vol->parent_disk->ops->read)
    {
        k_log("[DSK] Parent disk ops readnot initialized");
        return BLD_SIZE_ERR;
    }

    uint64_t actual_lba = lba + vol->lba_start;
    if (actual_lba > vol->lba_end)
    {
        k_log("[DSK] Requested out of bounds!");
        return BLD_0;
    }
    uint64_t actual_end = actual_lba + count;
    if (actual_end > vol->lba_end)
    {
        actual_end = vol->lba_end;
    }
    k_log("[DSK] Going to read!");
    return vol->parent_disk->ops->read(vol->parent_disk, actual_lba, actual_end - actual_lba, buffer);
}

int diskmanager_write_volume(disk_manager_volume_t *vol, uint64_t lba, uint32_t count, void *buffer)
{
    if (!vol)
    {
        k_log("[DSK] Volume not initialized!");
        return BLD_SIZE_ERR;
    }

    uint64_t actual_lba = lba + vol->lba_start;
    if (actual_lba > vol->lba_end)
        return BLD_0;
    uint64_t actual_end = actual_lba + count;
    if (actual_end > vol->lba_end)
    {
        actual_end = vol->lba_end;
    }
    return vol->parent_disk->ops->write(vol->parent_disk, actual_lba, actual_end - actual_lba, buffer);
}

int diskmanager_get_sector_count_volume(disk_manager_volume_t *vol)
{
    return vol->lba_end - vol->lba_start;
}