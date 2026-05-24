#pragma once
#include "process.h"
#include "task.h"

void scheduler_init();
uint8_t scheduler_tick(task_t **current, task_t **next);
void scheduler_add(task_t *process);
task_t *scheduler_get_current();
task_t *scheduler_get_next();
void scheduler_yield();
void scheduler_remove(task_t *task);
void scheduler_task_cleanupd();
void scheduler_enable();