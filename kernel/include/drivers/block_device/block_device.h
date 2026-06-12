#pragma once
#include "types.h"

#define BLD_SUCCESS 0
#define BLD_EINVAL -1
#define BLD_PROBE_FAIL -2
#define BLD_SIZE_ERR -1
#define BLD_SIZE_UNK -3
#define BLD_0 0;

void block_devices_init();

typedef struct block_drive block_drive_t;
typedef struct block_driver block_driver_t;

typedef int64 bld_size;

typedef struct block_driver
{
    const char *block_driver_name;
    int (*probe)(block_drive_t *this);
    int (*read)(block_drive_t *this, uint64_t lba, uint32_t count, void *buffer);
    int (*write)(block_drive_t *this, uint64_t lba, uint32_t count, void *buffer);
    bld_size (*get_sector_count)(block_drive_t *this);
} block_driver_t;


typedef struct block_drive
{
    const char *block_drive_name;
    block_driver_t* ops;
    void *private_fields;
} block_drive_t;


void block_devices_validate_and_add(block_drive_t *drives, uint16_t counts);