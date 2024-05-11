#include "locks_queues.h"
#include <stdio.h>



#define CHEESE1 0x4000
#define CHEESE2 0x4004


int __main() {

    lock_acquire(0);

    int x = rdcycle();
    int y = mhartid();
    printf("cheese %d %d\n", x, y);

    lock_release(0);

    for(;;) {
      asm volatile ("wfi");
    }
}


int main(void)
{
  // lock_acquire(0);
  printf("Starting potato test\n");
  // lock_release(0);
  lock_init(0);

  lock_acquire(0);
  printf("potato\n");
  lock_release(0);

  lock_acquire(0);
  printf("potato\n");
  lock_release(0);

  lock_acquire(0);
  printf("potato\n");
  lock_release(0);


  return 0;
}
