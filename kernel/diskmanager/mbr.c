#include "diskmanager.h"
#include "debug.h"
#include "kmalloc.h"
#include "kstring.h"
#include "string.h"

#define MBR_PARTITION_UNUSED 0x00
#define MBR_PARTITION_FAT_32 0x0B
#define MBR_PARTITION_FAT_32_LBA 0x0C

#define MBR_MAGIC_NUMBER_UNK_VALID 0xdead
#define MBR_MAGIC_NUMBER_VALID 0xf00d

typedef struct mbr_partition
{
    uint64_t start;
    uint64_t end;
    uint8_t is_valid;
} mbr_partition_t;

int16 diskmanager_mbr_check(char *first_sector, disk_manager_volume_t **out)
{
    k_log("[DSK][MBR] Checking for Master Boot Record Partitions");
    if (!first_sector)
        return DSK_INV;

    disk_manager_mbr_entry_t *entry = (disk_manager_mbr_entry_t *)(&first_sector[0x1BE]);

    disk_manager_volume_t *may_be_used_entries = (disk_manager_volume_t *)kmalloc(sizeof(disk_manager_volume_t) * 4);

    uint8_t may_be_used_entry_count = 0;

    for (int i = 0; i < 4; i++)
    {
        disk_manager_mbr_entry_t *p = &entry[i];
        k_log("[DSK][MBR] Looking at entry %d", i + 1);
        k_log("[DSK][MBR] Parition Type: %x", p->partition_type);
        k_log("[DSK][MBR] Partition Start: %d", p->lba_partition_start);
        k_log("[DSK][MBR] Num Sectors: %d", p->lba_num_sectors);

        switch (p->partition_type)
        {
        case MBR_PARTITION_UNUSED:
            k_log("[DSK][MBR] May be MBR Unused partition, skipping!");
            break;
        case MBR_PARTITION_FAT_32:
        case MBR_PARTITION_FAT_32_LBA:
            k_log("[DSK][MBR] Probably FAT32 partition!");
            
            k_log("PTR 0x%x", may_be_used_entries);
            k_log("sizeof(volume)=%d", sizeof(disk_manager_volume_t));
            k_log("PTR 0x%x", &may_be_used_entries[1]);
            may_be_used_entries[may_be_used_entry_count++] = (disk_manager_volume_t){
                .lba_start = p->lba_partition_start,
                .lba_end = p->lba_partition_start + p->lba_num_sectors,
                .type = VOL_UNK_VALID,
                .id = -1,
                .parent_disk = NULL,
                .magic_number = MBR_MAGIC_NUMBER_VALID
            };

            k_log("[DSK] Added");
            break;
        default:
            k_log("[DSK][MBR] Unknown Partition type!");
            may_be_used_entries[may_be_used_entry_count++] = (disk_manager_volume_t){
                .lba_start = p->lba_partition_start,
                .lba_end = p->lba_partition_start + p->lba_num_sectors,
                .type = VOL_UNK,
                .parent_disk = NULL,
                .id = -1,
                .magic_number = MBR_MAGIC_NUMBER_UNK_VALID,
            };
            break;
        }
    }

    if (may_be_used_entry_count <= 0)
    {
        k_log("[DSK][MBR] No possible valid entries found, skipping!");
        return DSK_INV;
    }
    k_log("[DSK][MBR] found probable %d partitions", may_be_used_entry_count);

    for (int i = 0; i < may_be_used_entry_count; i++)
    {
        if (may_be_used_entries[i].lba_end - may_be_used_entries[i].lba_start <= 0)
        {
            k_log("[DSK][MBR] found an entry with size less than 0, disk invalid");
            return DSK_INV;
        }
    }

    k_log("[DSK][MBR] Checking for overlap");
    for (int i = 0; i < may_be_used_entry_count; i++)
    {
        for (int j = 0; j < i; j++)
        {
            if (may_be_used_entries[i].lba_start < may_be_used_entries[j].lba_start)
            {
                if (may_be_used_entries[i].lba_end > may_be_used_entries[j].lba_start)
                {
                    return DSK_INV;
                }
            }
            else
            {
                if (may_be_used_entries[j].lba_end > may_be_used_entries[i].lba_start)
                {
                    return DSK_INV;
                }
            }
        }
    }

    k_log("[MBR] I think the partition is valid! Found %d.", may_be_used_entry_count);
    *out = may_be_used_entries;
    return may_be_used_entry_count;
}