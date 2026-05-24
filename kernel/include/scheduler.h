#pragma once
#include "process.h"
#include "task.h"

void scheduler_init();
uint8_t scheduler_tick(task_t **current, task_t **next);
void scheduler_add(task_t *process);
task_t *scheduler_get_current();
void scheduler_yield();