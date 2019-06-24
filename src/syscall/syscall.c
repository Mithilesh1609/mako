
// syscall.c
//
// System calls.
//
// Author: Ajay Tatachar <ajaymt2@illinois.edu>

#include <process/process.h>
#include <interrupt/interrupt.h>
#include <fs/fs.h>
#include <klock/klock.h>
#include <kheap/kheap.h>
#include <debug/log.h>
#include "syscall.h"

void syscall_fork()
{
  process_t *current = process_current();
  process_t *child = kmalloc(sizeof(process_t));
  process_fork(child, current);
  child->regs.eax = 0;
  current->regs.eax = child->pid;
  process_schedule(child);
}

void syscall_execve(char *path, char *argv[], char *envp[])
{
  // TODO.
}

void syscall_exit(uint32_t status)
{
  process_finish(process_current());
  process_switch_next();
}

static volatile uint32_t a = 0;

process_registers_t *syscall_handler(cpu_state_t cs, stack_state_t ss)
{
  //update_current_process_registers(cs, ss);
  //log_debug("syscall", "syscall\n");

  klock(&a);
  for (uint32_t i = 0; i < (uint32_t)-1; ++i);
  kunlock(&a);

  while(1);

  return &(process_current()->regs);
}
