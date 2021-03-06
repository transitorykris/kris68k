/*
Copyright 2020 Kris Foster

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef CONTEXT_H
#define CONTEXT_H

#include "sys/errors.h"
#include "signal.h"

#include "users.h"

#define EMBRYO      1
#define RUNNING     2
#define SLEEPING    3
#define ZOMBIE      0

#define MAX_CHILDREN    64
#define MAX_WAIT_LIST   64

typedef uint8_t state_t;

struct pcb_t {
    // Order matters here
    uint32_t d[8];          // Data registers D1-D7
    uint32_t a[7];          // Address registers A0-A6
    uint32_t *usp;          // User stack pointer / A7
    uint32_t sr;            // Status register
    uint32_t pc;            // Program counter

    struct pcb_t* next; // Next process to run

    state_t state;          // State of this process
    error_t _errno;         // Last error reported by the kernel
    uint32_t *stack_base;   // The highest address of this context's stack

    sig_t signal_handler[SIGNAL_COUNT];  // pointers to sig handler funcs

    pid_t pid;              // Index for this task in the process table
    pid_t parent;
    pid_t children[MAX_CHILDREN];   // List of all children spawned
    pid_t wait_list[MAX_WAIT_LIST]; // Processes waiting on this process to exit
};

extern void context_swap();
extern void context_init();
extern void context_save();
extern void context_restore();

#endif