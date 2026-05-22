#include "process.h"
#include "kmalloc.h"
#include "string.h"
#include "debug.h"


void process_destroy(process_t *process)
{
    k_log("[PROCESS] Destroying process with pid %d", process->pid);
    vmm_destroy(process->address_space);
    kfree((void *)(process->stack_base));
    process_table[process->pid] = NULL;
    kfree((void *)process);
}
