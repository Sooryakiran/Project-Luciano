#pragma once
#include "block_device.h"
#include "types.h"

#define DSK_MAX_VOLUMES 16
#define DSK_INV -1
#define DSK_OK 0

typedef enum disk_manager_volume_type {
    VOL_UNK,
    VOL_UNK_VALID,
    VOL_FAT32
} disk_manager_volume_type_t;

typedef struct disk_manager_volume {
    uint64_t lba_start;
    uint64_t lba_end;
    disk_manager_volume_type_t type; 
    block_drive_t *parent_disk;
    int16 id;
    uint16_t magic_number;
} disk_manager_volume_t;


typedef struct disk_manager_mbr_entry {
    uint8_t drive_attributes;
    uint8_t chs_partition_start[3];
    uint8_t partition_type;
    uint8_t chs_partition_end[3];
    uint32_t lba_partition_start;
    uint32_t lba_num_sectors;
} __attribute__((packed)) disk_manager_mbr_entry_t;

void diskmanager_init();
void diskmanager_setup_drive(block_drive_t*);

int16 diskmanager_mbr_check(char *first_sector, disk_manager_volume_t**out);

void diskmanager_setup_partitions(disk_manager_volume_t *);

int diskmanager_read_volume(disk_manager_volume_t* vol, uint64_t lba, uint32_t count, void *buffer);
int diskmanager_write_volume(disk_manager_volume_t* vol, uint64_t lba, uint32_t count, void *buffer);
int diskmanager_get_sector_count_volume(disk_manager_volume_t *vol);