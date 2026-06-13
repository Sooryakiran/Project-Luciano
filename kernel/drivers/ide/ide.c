#include "drivers/ide/ide.h"
#include "block_device.h"
#include "debug.h"
#include "io.h"
#include "string.h"

#define IDE_PRIMARY_BASE 0x1F0
#define IDE_SECONDARY_BASE 0x170
#define IDE_MASTER 0xA0
#define IDE_SLAVE 0xB0

#define IDE_SECTOR_COUNT_OFFSET 2
#define IDE_LBA_LOW_OFFSET 3
#define IDE_LBA_MID_OFFSET 4
#define IDE_LBA_HIGH_OFFSET 5
#define IDE_DRIVE_SELECT_OFFSET 6
#define IDE_STATUS_OFFSET 7
#define IDE_COMMAND_OFFSET 7

#define IDE_STATUS_BSY 0x80  // Busy
#define IDE_STATUS_DRDY 0x40 // Drive ready
#define IDE_STATUS_DF 0x20   // Drive fault
#define IDE_STATUS_DSC 0x10  // Disk seek complete
#define IDE_STATUS_DRQ 0x08  // Data request
#define IDE_STATUS_CORR 0x04 // Corrected data
#define IDE_STATUS_INDEX 0x02
#define IDE_STATUS_ERR 0x01 // Error

#define IDE_COMMAND_IDENTIFY 0xEC
#define IDE_COMMAND_READ 0x20

#define IDE_IDENTIFY_RESULT_SIZE 256
#define IDE_WORDS_PER_SECTOR 256
#define IDE_DRIVER_NAME "ide_driver_soorkie"

typedef struct ide_private_field
{
    uint16_t channel;
    uint16_t drive;
    bld_size sector_count;
} ide_private_field_t;

static ide_private_field_t all_drive_configs[] = {
    {
        .channel = IDE_PRIMARY_BASE,
        .drive = IDE_MASTER,
        .sector_count = BLD_SIZE_UNK,
    },
    {
        .channel = IDE_PRIMARY_BASE,
        .drive = IDE_SLAVE,
        .sector_count = BLD_SIZE_UNK,
    },
    {
        .channel = IDE_SECONDARY_BASE,
        .drive = IDE_MASTER,
        .sector_count = BLD_SIZE_UNK,
    },
    {
        .channel = IDE_SECONDARY_BASE,
        .drive = IDE_SLAVE,
        .sector_count = BLD_SIZE_UNK,
    }};

static block_driver_t ops = {
    .block_driver_name = IDE_DRIVER_NAME,
    .read = &ide_read,
    .write = NULL,
    .probe = &ide_probe,
    .get_sector_count = &ide_get_sector_count,
};

static block_drive_t all_drives[] = {
    {.block_drive_name = "hda",
     .ops = &ops,
     .logical_sector_size = 512, // hard coding for now
     .private_fields = (void *)&all_drive_configs[0]},
    {.block_drive_name = "hdb",
     .ops = &ops,
     .logical_sector_size = 512,
     .private_fields = (void *)&all_drive_configs[1]},
    {.block_drive_name = "hdc",
     .ops = &ops,
     .logical_sector_size = 512,
     .private_fields = (void *)&all_drive_configs[2]},
    {.block_drive_name = "hdd",
     .ops = &ops,
     .logical_sector_size = 512,
     .private_fields = (void *)&all_drive_configs[3]}};

void ide_init()
{
    k_log("[IDE] Ide Init");
    k_log("[IDE] Done!");
}

void ide_delay(uint16_t channel)
{
    inb(channel + 0x206);
    inb(channel + 0x206);
    inb(channel + 0x206);
    inb(channel + 0x206);
}

void ide_reset(uint16_t channel)
{
    outb(channel + 0x206, 0x04);
    for (int i = 0; i < 20; i++)
        ide_delay(channel);
    outb(channel + 0x206, 0x00);
    while (inb(channel + 0x206) & 0x80);
}

int ide_probe(block_drive_t *this)
{
    if (!this)
    {
        k_log("[IDE] Instance of driver not initialized!");
        return BLD_EINVAL;
    }

    if (!this->ops)
    {
        k_log("[IDE] Instance of driver ops not initialized!");
        return BLD_EINVAL;
    }

    if (strcmp(this->ops->block_driver_name, IDE_DRIVER_NAME) != 0)
    {
        k_log("[IDE] Invalid driver being called %s", this->block_drive_name);
        return BLD_EINVAL;
    }

    ide_private_field_t *private = (ide_private_field_t *)this->private_fields;
    if (!private)
    {
        k_log("[IDE] Private fields are corrupt like people");
        return BLD_EINVAL;
    }

    outb(private->channel + IDE_DRIVE_SELECT_OFFSET, private->drive);
    // Small delay (optional)
    // for (int i = 0; i < 1000; i++)
    //     continue;
    ide_delay(private->channel);

    uint8_t status = inb(private->channel + IDE_STATUS_OFFSET);

    k_log("[IDE] Probe status is %x", status);
    if (status == 0xFF || status == 0x00)
    {
        k_log("[IDE] Drive not available, status is 0x%x", status);
        return BLD_PROBE_FAIL; // No drive
    }
    k_log("[IDE] Drive available");

    return BLD_SUCCESS;
}

bld_size ide_get_sector_count(block_drive_t *this)
{
    if (!this || !this->ops)
    {
        k_log("[IDE] Instance of driver not initialized!");
        return BLD_SIZE_ERR;
    }

    ide_private_field_t *private_fields = this->private_fields;
    if (!private_fields)
    {
        k_log("[IDE] Private fields are corrupt like people");
        return BLD_SIZE_ERR;
    }

    if (private_fields->sector_count != BLD_SIZE_UNK)
    {
        k_log("[IDE] Reading sector count from cache");
        return private_fields->sector_count;
    }

    if (strcmp(this->ops->block_driver_name, IDE_DRIVER_NAME) != 0)
    {
        k_log("[IDE] Invalid driver being called %s", this->block_drive_name);
        return BLD_SIZE_ERR;
    }

    ide_private_field_t *private = (ide_private_field_t *)this->private_fields;
    if (!private)
    {
        k_log("[IDE] Private fields are corrupt like people");
        return BLD_SIZE_ERR;
    }

    uint8_t drive_select_val = 0xE0 | (private_fields->drive);
    outb(private->channel + IDE_DRIVE_SELECT_OFFSET, drive_select_val);
    // Small delay (optional)
    ide_delay(private->channel);

    outb(private->channel + IDE_COMMAND_OFFSET, IDE_COMMAND_IDENTIFY);

    k_log("[IDE] Waiting for IDE drive to respond status");
    while (1)
    {
        uint8_t status = inb(private_fields->channel + IDE_STATUS_OFFSET);
        if (status & IDE_STATUS_ERR)
        {
            k_log("[IDE] Drive not available error! Status code 0x%x", status);
            ide_reset(private->channel);
            return BLD_SIZE_ERR;
        }
        // Safe to read only when NOT busy AND data request is active
        if (!(status & IDE_STATUS_BSY) && (status & IDE_STATUS_DRQ))
        {
            break;
        }
    }

    if (inb(private->channel + IDE_STATUS_OFFSET) & IDE_STATUS_ERR)
    {
        k_log("[IDE] IDE identification failed miserably");
        ide_reset(private->channel);
        return BLD_SIZE_ERR;
    }

    uint16_t buffer[IDE_IDENTIFY_RESULT_SIZE];
    for (int i = 0; i < IDE_IDENTIFY_RESULT_SIZE; i++)
    {
        buffer[i] = inw(private->channel);
    }

    // we do 28 bit lba only for now
    // todo 48 bit lba future

    uint64_t sectors = ((uint64_t)buffer[61] << 16) | ((uint64_t)buffer[60]);
    if (sectors <= 0)
        return BLD_SIZE_ERR;

    k_log("[IDE] sectors found %d for %s", sectors, this->block_drive_name);
    private_fields->sector_count = sectors;
    return sectors;
}

int ide_read(block_drive_t *this, uint64_t lba, uint32_t count, void *buffer)
{
    if (!this || !this->ops)
    {
        k_log("[IDE] Instance of driver not initialized!");
        return BLD_EINVAL;
    }

    bld_size sector_size = this->ops->get_sector_count(this);
    if (lba > sector_size)
        return BLD_0;

    uint64_t limit = count;
    if (count + lba > sector_size)
        limit = sector_size - lba;

    // hard limitation
    if (limit >= 256)
        limit = 255;

    ide_private_field_t *private_fields = (ide_private_field_t *)this->private_fields;
    if (!private_fields)
        return BLD_EINVAL;

    uint8_t drive_select_val = 0xE0 | (private_fields->drive) | ((lba >> 24) & 0x0F);

    outb(private_fields->channel + IDE_DRIVE_SELECT_OFFSET, drive_select_val);
    outb(private_fields->channel + IDE_SECTOR_COUNT_OFFSET, limit);

    // 28 bit only
    outb(private_fields->channel + IDE_LBA_LOW_OFFSET, lba & 0xFF);
    outb(private_fields->channel + IDE_LBA_MID_OFFSET, (lba >> 8) & 0xFF);
    outb(private_fields->channel + IDE_LBA_HIGH_OFFSET, (lba >> 16) & 0xFF);

    outb(private_fields->channel + IDE_COMMAND_OFFSET, IDE_COMMAND_READ);

    k_log("[IDE] Private fields are %x and %x", private_fields->channel, private_fields->drive);
    k_log("[IDE] Waiting for IDE drive to respond status");
    while (1)
    {
        uint8_t status = inb(private_fields->channel + IDE_STATUS_OFFSET);
        if (status & IDE_STATUS_ERR)
        {
            k_log("[IDE] Drive read error!");
            ide_reset(private_fields->channel);
            return BLD_SIZE_ERR;
        }
        // Safe to read only when NOT busy AND data request is active
        if (!(status & IDE_STATUS_BSY) && (status & IDE_STATUS_DRQ))
        {
            break;
        }
    }

    if (inb(private_fields->channel + IDE_STATUS_OFFSET) & IDE_STATUS_ERR)
    {
        k_log("[IDE] IDE read failed pathetically");
        ide_reset(private_fields->channel);
        return BLD_SIZE_ERR;
    }

    uint16_t *bufferw = buffer;
    for (int i = 0; i < limit; i++)
    {
        while (1)
        {
            uint8_t status = inb(private_fields->channel + IDE_STATUS_OFFSET);
            if (status & IDE_STATUS_ERR)
                return BLD_SIZE_ERR;
            if (!(status & IDE_STATUS_BSY) && (status & IDE_STATUS_DRQ))
                break;
        }
        for (int j = 0; j < IDE_WORDS_PER_SECTOR; j++)
        {
            int idx = i * IDE_WORDS_PER_SECTOR + j;
            bufferw[idx] = inw(private_fields->channel);
            // k_log("READ (%d, %d) value -> %d", i, j, bufferw[idx]);
        }
    }
    return limit;
}

block_drive_t *ide_get_drives()
{
    return all_drives;
}

uint16_t ide_get_drive_count()
{
    return 4;
}
