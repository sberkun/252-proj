#include <stdio.h>
#include "locks_queues.h"



// volatile int spud = 0;
volatile int start_flag = 0;
volatile int done_flags[4] = {0,0,0,0};

// void increment_spud() {
//     lock_acquire(0);
//     // printf("ss\n"); // TODO: comment out
//     spud += 1;
//     if (spud == 100) {
//         done_flag = 1;
//     }
//     lock_release(0);
// }


void do_thing() {
    for (int i = 0; i < 25; i++) {
        lock_acquire(0);
        lock_release(0);
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
  printf("Starting potato2 test\n");

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
