#include <stdio.h>
#include "locks_queues.h"



void do_thing() {
    for (int i = 0; i < 200; i++) {
        monitor_acquire(0,0);
        monitor_release(0);
    }
}

int __main() {
    for(;;) {
      asm volatile ("wfi");
    }
}

int main(void)
{
  printf("Starting potato4 test\n");

  lock_init(0);
  int t1 = rdcycle();

  do_thing();

  int t2 = rdcycle();

  printf("done! %d %d %d\n", t1, t2, t2 - t1);

  return 0;
}
