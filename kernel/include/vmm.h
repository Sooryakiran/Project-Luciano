#pragma once
#include "types.h"

typedef uint64_t vmm_page_entry;

void vmm_map(vmm_page_entry*, vaddr_t, paddr_t, uint8_t);
paddr_t vmm_get_paddr(vmm_page_entry*, vaddr_t);
void vmm_init(paddr_t, vaddr_t, uint64_t);
void vmm_unmap(vmm_page_entry*, vaddr_t);
vmm_page_entry * vmm_get_current_pml4();
vmm_page_entry *vmm_create_pml4();
void vmm_switch(vmm_page_entry *);