#pragma once
#include "types.h"
#include "drivers/block_device/block_device.h"


void ide_init();
int ide_probe(block_drive_t *this);
int ide_read(block_drive_t *this, uint64_t lba, uint32_t count, void *buffer);
bld_size ide_get_sector_count(block_drive_t *this);

block_drive_t *ide_get_drives();
uint16_t ide_get_drive_count();