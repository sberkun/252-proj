#include <stdio.h>
#include "locks_queues.h"



// volatile int spud = 0;
volatile int start_flag = 0;
volatile int done_flags[4] = {0,0,0,0};



volatile int locked = 0;
void atomic_lock_acquire() {
    volatile int* lock_addr = &locked;
    asm volatile (
        "li t0, 1\n"
      "123:\n"
        "lw t1, (%0)\n"
        "bnez t1, 123b\n"
        "amoswap.w.aq t1, t0, (%0)\n"
        "bnez t1, 123b\n"
      : // outputs
      : "r"(lock_addr) // inputs
      : "t0", "t1" // clobbers
    );
}

void atomic_lock_release() {
  volatile int* lock_addr = &locked;
  asm volatile (
    "amoswap.w.rl x0, x0, (%0)"
    :
    : "r"(lock_addr)
  );
}


void do_thing() {
    for (int i = 0; i < 25; i++) {
        atomic_lock_acquire();
        atomic_lock_release();
    }
    done_flags[mhartid()] = 1;
}

int __main() {
    while (!start_flag);

    do_thing();

    for(;;) {
      asm volatile ("wfi");
    }
}

int main(void)
{
  printf("Starting potato3 test\n");

  lock_init(0);
  int t1 = rdcycle();
  start_flag = 1;

  do_thing();

  for (int i = 1; i < 4; i++) {
    while (!done_flags[i]);
  }
  int t2 = rdcycle();

  printf("done! %d %d\n", t1, t2);


  return 0;
}
