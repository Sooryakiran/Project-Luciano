#include "pmm.h"
#include "debug.h"
#include "string.h"
#include "vmm.h"

#define PML4_IDX(va) (((va) >> 39) & 0x1FF)
#define PDPT_IDX(va) (((va) >> 30) & 0x1FF)
#define PD_IDX(va) (((va) >> 21) & 0x1FF)
#define PT_IDX(va) (((va) >> 12) & 0x1FF)
#define ENTRY_PRESENT(entry) ((entry) & 0x1)
#define ADDRESS_MASK 0x000FFFFFFFFFF000ULL
#define VMM_FAIL 0x0

#define K_PAGE_START 256
#define K_PAGE_END 512
#define PAGE_ENTRIES 512

// https://wiki.osdev.org/Paging#Page_Map_Table_Entries
// https://wiki.osdev.org/Page_Tables

// TODO: assumption on page size 4096 and frame size 4096 equal in pmm_alloc()

extern char __bss_end;

static uint64_t vmm_hhdm_offset;

typedef uint64_t vmm_page_entry;

void vmm_init(
    paddr_t kernel_physical_addr,
    vaddr_t kernel_virtual_addr,
    uint64_t hhdm_offset)
{

    k_log("[VMM] Initializing paging...");
    vmm_hhdm_offset = hhdm_offset;

    vmm_page_entry *pml4 = (vmm_page_entry *)(pmm_alloc() + hhdm_offset);
    memset(pml4, 0, PMM_FRAME_SIZE);

    uint64_t total_memory = pmm_get_total_memory();
    k_log("[VMM] Obtained total memory size as %lu", total_memory);
    for (uint64_t phys = 0; phys < total_memory; phys += PMM_FRAME_SIZE)
    {
        vmm_map(pml4, phys + hhdm_offset, phys, 0x3);
    }
    k_log("[VMM] Mapped all memory hhdm offset to physical ram");

    uint64_t kernel_virtual_end = (uint64_t)(&__bss_end);
    uint64_t kernel_size = kernel_virtual_end - kernel_virtual_addr;
    k_log("[VMM] Kernel virtual start is %lu", kernel_virtual_addr);
    k_log("[VMM] Kernel virtual end is %lu", kernel_virtual_end);
    k_log("[VMM] Kernel size is %lu", kernel_size);

    for (uint64_t offset = 0; offset < kernel_size; offset += PMM_FRAME_SIZE)
    {
        vmm_map(pml4, kernel_virtual_addr + offset, kernel_physical_addr + offset, 0x3);
    }

    k_log("[VMM] Kernel mapping done!");
    vmm_switch(pml4);
    k_log("[VMM] Paging Initialized");
}

address_space_t vmm_create()
{
    k_log("[VMM] Creating new PML4");
    vmm_page_entry *pml4 = (vmm_page_entry *)(pmm_alloc() + vmm_hhdm_offset);
    memset(pml4, 0, PMM_FRAME_SIZE);
    vmm_page_entry *current_pml4 = vmm_get_current_space();

    memcpy(&pml4[K_PAGE_START], &current_pml4[K_PAGE_START], (K_PAGE_END - K_PAGE_START) * sizeof(vmm_page_entry));
    return (address_space_t)pml4;
}

address_space_t vmm_get_current_space()
{
    uint64_t phys;
    asm volatile("mov %%cr3, %0" : "=r"(phys));
    return (address_space_t)(vmm_page_entry *)(phys + vmm_hhdm_offset);
}

void vmm_map(address_space_t address_space, vaddr_t virtual_addr, paddr_t physical_addr, uint8_t flags)
{
    vmm_page_entry *pml4 = (vmm_page_entry *)address_space;
    uint64_t pml4_idx = PML4_IDX(virtual_addr);
    uint64_t pdpt_idx = PDPT_IDX(virtual_addr);
    uint64_t pd_idx = PD_IDX(virtual_addr);
    uint64_t pt_idx = PT_IDX(virtual_addr);

    // PML4 walking
    vmm_page_entry pml4_entry = pml4[pml4_idx];
    if (!ENTRY_PRESENT(pml4_entry))
    {
        // address is already 4kb aligned (first 12 bits 0)
        // 0x3 present and RW flag
        // TODO: change flags in userspace mappings
        pml4[pml4_idx] = pmm_alloc() | 0x3;
    }

    // PDPT walking
    vmm_page_entry *pdpt = (vmm_page_entry *)((pml4[pml4_idx] & ADDRESS_MASK) + vmm_hhdm_offset);
    vmm_page_entry pdpt_entry = pdpt[pdpt_idx];
    if (!ENTRY_PRESENT(pdpt_entry))
    {
        pdpt[pdpt_idx] = pmm_alloc() | 0x3;
    }

    // PD walking
    vmm_page_entry *pd = (vmm_page_entry *)((pdpt[pdpt_idx] & ADDRESS_MASK) + vmm_hhdm_offset);
    vmm_page_entry pd_entry = pd[pd_idx];
    if (!ENTRY_PRESENT(pd_entry))
    {
        pd[pd_idx] = pmm_alloc() | 0x3;
    }

    // PT walking
    vmm_page_entry *pt = (vmm_page_entry *)((pd[pd_idx] & ADDRESS_MASK) + vmm_hhdm_offset);
    vmm_page_entry pt_entry = pt[pt_idx];
    if (!ENTRY_PRESENT(pt_entry))
    {
        // finally
        pt[pt_idx] = physical_addr | flags;
    }
}

paddr_t vmm_get_paddr(address_space_t address_space, vaddr_t virtual_addr)
{
    vmm_page_entry *pml4 = (vmm_page_entry *)address_space;
    uint64_t pml4_idx = PML4_IDX(virtual_addr);
    uint64_t pdpt_idx = PDPT_IDX(virtual_addr);
    uint64_t pd_idx = PD_IDX(virtual_addr);
    uint64_t pt_idx = PT_IDX(virtual_addr);

    vmm_page_entry pml4_entry = pml4[pml4_idx];
    if (!ENTRY_PRESENT(pml4_entry))
        return VMM_FAIL;

    vmm_page_entry *pdpt = (vmm_page_entry *)((pml4_entry & ADDRESS_MASK) + vmm_hhdm_offset);
    vmm_page_entry pdpt_entry = pdpt[pdpt_idx];
    if (!ENTRY_PRESENT(pdpt_entry))
        return VMM_FAIL;

    vmm_page_entry *pd = (vmm_page_entry *)((pdpt_entry & ADDRESS_MASK) + vmm_hhdm_offset);
    vmm_page_entry pd_entry = pd[pd_idx];
    if (!ENTRY_PRESENT(pd_entry))
        return VMM_FAIL;

    vmm_page_entry *pt = (vmm_page_entry *)((pd_entry & ADDRESS_MASK) + vmm_hhdm_offset);
    vmm_page_entry pt_entry = pt[pt_idx];

    return pt_entry & ADDRESS_MASK;
}

void vmm_switch(address_space_t address_space)
{
    vmm_page_entry *pml4 = (vmm_page_entry *)address_space;
    uint64_t physical_addr = (uint64_t)pml4 - vmm_hhdm_offset;
    asm volatile(
        "mov %0, %%cr3"      // %0 = first input, %% escapes to literal % for registers
        :                    // output operands (none)
        : "r"(physical_addr) // input: "r" = put in any general purpose register, (phys) = the value
        : "memory"           // clobber: tell compiler memory may have changed
    );
}

void vmm_unmap(address_space_t address_space, vaddr_t virtual_addr)
{
    vmm_page_entry *pml4 = (vmm_page_entry *)address_space;
    uint64_t pml4_idx = PML4_IDX(virtual_addr);
    uint64_t pdpt_idx = PDPT_IDX(virtual_addr);
    uint64_t pd_idx = PD_IDX(virtual_addr);
    uint64_t pt_idx = PT_IDX(virtual_addr);

    vmm_page_entry pml4_entry = pml4[pml4_idx];
    if (!ENTRY_PRESENT(pml4_entry))
        return;

    vmm_page_entry *pdpt = (vmm_page_entry *)((pml4_entry & ADDRESS_MASK) + vmm_hhdm_offset);
    vmm_page_entry pdpt_entry = pdpt[pdpt_idx];
    if (!ENTRY_PRESENT(pdpt_entry))
        return;

    vmm_page_entry *pd = (vmm_page_entry *)((pdpt_entry & ADDRESS_MASK) + vmm_hhdm_offset);
    vmm_page_entry pd_entry = pd[pd_idx];
    if (!ENTRY_PRESENT(pd_entry))
        return;

    vmm_page_entry *pt = (vmm_page_entry *)((pd_entry & ADDRESS_MASK) + vmm_hhdm_offset);
    vmm_page_entry pt_entry = pt[pt_idx];

    pt[pt_idx] = 0;
    asm volatile("invlpg (%0)" ::"r"(virtual_addr) : "memory");
    return;
}

void vmm_destroy(address_space_t address_space)
{
    vmm_page_entry *pml4 = (vmm_page_entry *)address_space;

    for (uint16_t pml4_idx = 0; pml4_idx < K_PAGE_START; pml4_idx++)
    {
        if (!ENTRY_PRESENT(pml4[pml4_idx]))
            continue;
        vmm_page_entry *pdpt = (vmm_page_entry *)((pml4[pml4_idx] & ADDRESS_MASK) + vmm_hhdm_offset);
        for (uint16_t pdpt_idx = 0; pdpt_idx < PAGE_ENTRIES; pdpt_idx++)
        {
            if (!ENTRY_PRESENT(pdpt[pdpt_idx]))
                continue;
            vmm_page_entry *pd = (vmm_page_entry *)((pdpt[pdpt_idx] & ADDRESS_MASK) + vmm_hhdm_offset);
            for (uint16_t pd_idx = 0; pd_idx < PAGE_ENTRIES; pd_idx++)
            {
                if (!ENTRY_PRESENT(pd[pd_idx]))
                    continue;
                vmm_page_entry *pt = (vmm_page_entry *)((pd[pd_idx] & ADDRESS_MASK) + vmm_hhdm_offset);
                for (uint16_t offset = 0; offset < PAGE_ENTRIES; offset++)
                {
                    if (!ENTRY_PRESENT(pt[offset]))
                        continue;
                    pmm_free(pt[offset] & ADDRESS_MASK);
                }
                pmm_free(pd[pd_idx] & ADDRESS_MASK);
            }
            pmm_free(pdpt[pdpt_idx] & ADDRESS_MASK);
        }
        pmm_free(pml4[pml4_idx] & ADDRESS_MASK);
    }
    pmm_free((paddr_t)((uint64_t)address_space - vmm_hhdm_offset));
}