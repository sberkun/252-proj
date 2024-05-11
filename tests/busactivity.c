#include <stdio.h>
#include "locks_queues.h"



// volatile int spud = 0;
volatile int start_flag = 0;
volatile int done_flag = 0;


#define AR_SIZE 1024
#define LOCK_AMOUNT 10000

int large_array_1[AR_SIZE];
int large_array_2[AR_SIZE];
int large_array_3[AR_SIZE];


int __main() {
    while (!start_flag);

    for (int i = 0; i < LOCK_AMOUNT; i++) {
        lock_acquire(0);
        lock_release(0);
    }
    done_flag = 1;

    for(;;) {
      asm volatile ("wfi");
    }
}

int main(void)
{
  printf("Starting bus test\n");

  lock_init(0);
  done_flag = 0;
  start_flag = 1;

  int t1 = rdcycle();
  for (int a = 0; a < AR_SIZE; a++) {
    large_array_3[a] = large_array_1[a] + large_array_2[a];
  }
  int t2 = rdcycle();

  printf("other cores done: %d\n", done_flag);
  printf("done! %d %d\n", t1, t2);


  return 0;
}
