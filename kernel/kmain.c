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

#include <stdlib.h>
#include <debug_stub.h>
#include <easy68k/easy68k.h>

#include "helpers.h"
#include "context.h"
#include "system.h"
#include "mfp.h"

typedef uint8_t state;
#define RUNNING 0
#define DEAD 1

// Order matters! Append only
struct context_t {
    uint32_t d[7];      // Data registers D1-D7
    uint32_t a[6];      // Address registers A1-A6, a7 will be stored in usp
    uint32_t d0;        // Saved separately from ^
    uint32_t a0;        // Ditto
    uint32_t usp;       // User stack pointer
    uint16_t sr;        // Status register (and really just CCR)
    uint32_t pc;        // Program counter
    struct context_t* next;     // Next process to run

    // Order shouldn't matter too much below this line
    //bool running;
};

uint32_t scratch;

struct context_t *current_process;

void tick_handler();

void user_routine_a();
void pid0();

noreturn void kmain() {
  debug_stub();

  for (int i=0;i<50000;i++) {/* do nothing for a while */}

  e68ClearScr();
  e68Println("Kernel started");

  // pid0 will always be defined so the system has something to do  
  struct context_t pid0_context = {
    .usp = 0x6000,
    //.running = true
  };
  pid0_context.next = &pid0_context;
  current_process = &pid0_context;

  // Start the timer for firing the scheduler
  SET_VECTOR(context_swap, MFP_TIMER_C);

  // We need to set up USP before disabling supervisor mode
  // or we'll get a privilege error
  //register uint32_t *a0 __asm__ ("a0") __attribute__((unused));
  //a0 = (uint32_t *)current_process->usp;
  //__asm__ __volatile__ ("move.l %%a0,%%usp":::);

  disable_supervisor();

  pid0();
}

/*
User space routine that doesn't do too much
Same as the first
*/
noreturn void pid0() {
  //__asm__ __volatile (
  //  "move.l #0xf0f0f0f0,%d4"
  //);
  //e68Println("PID0 sleep forever");
  for (;;) {
    e68DisplayNumUnsigned((uint32_t)get_ticks(),10);
    e68Println(" TICK"); // this will go away once things work
  }
}

/*
User space routine that doesn't do too much
*/
void user_routine_a() {
  e68Println("Start of routine_a");
  for (;;) {
    e68Println("A");
  }
}
