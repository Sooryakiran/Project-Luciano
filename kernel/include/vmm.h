#pragma once
#include "types.h"

typedef void* address_space_t;

void vmm_init(paddr_t, vaddr_t, uint64_t);
void vmm_map(address_space_t, vaddr_t, paddr_t, uint8_t);
void vmm_unmap(address_space_t, vaddr_t);
paddr_t vmm_get_paddr(address_space_t, vaddr_t);
address_space_t vmm_get_current_space();
address_space_t vmm_create();
void vmm_switch(address_space_t);