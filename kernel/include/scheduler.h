#pragma once
#include "process.h"

void scheduler_init();
uint8_t scheduler_tick(process_t **current, process_t **next);
void scheduler_start();
void scheduler_add(process_t *process);