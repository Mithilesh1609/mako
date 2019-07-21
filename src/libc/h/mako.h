
// mako.h
//
// Mako specific syscalls.
//
// Author: Ajay Tatachar <ajaymt2@illinois.edu>

#ifndef _MAKO_H_
#define _MAKO_H_

#include <stdint.h>
#include <sys/types.h>

typedef void (*thread_t)(void *);

int32_t pipe(uint32_t *readfd, uint32_t *writefd);
int32_t movefd(uint32_t from, uint32_t to);
uint32_t pagealloc(uint32_t npages);
int32_t pagefree(uint32_t vaddr, uint32_t npages);
pid_t thread(thread_t t, void *data);
int32_t msleep(uint32_t duration);

void _init_thread();

#endif /* _MAKO_H_ */
