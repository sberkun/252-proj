#include "mmio.h"

#define LOCK_BASE 0x4000
#define NUM_LOCKS 8

// initialize lock to unlocked state
inline void lock_init(int lock_num) {
  uintptr_t lock_addr = LOCK_BASE + lock_num << 2;
  reg_write8(lock_addr, 0);
}

inline void lock_acquire(int lock_num) {
  uintptr_t lock_addr = LOCK_BASE + lock_num << 2;
  uint8_t ack;
  do {
    ack = reg_read8(lock_addr);
  } while(ack == 0);
}

inline void lock_release(int lock_num) {
  uintptr_t lock_addr = LOCK_BASE + lock_num << 2;
  reg_write8(lock_addr, 0);
}


#define QUEUE_BASE 0x4020
#define QUEUE_DEPTH 4
#define NUM_QUEUES 4

// initialize queue to empty state
inline void queue_init(int queue_num) {
  uintptr_t queue_addr = QUEUE_BASE + queue_num << 3;
  for (int a = 0; a < QUEUE_DEPTH; a++) {
    reg_read32(queue_addr);
  }
}

// pops some nonzero data from the queue
inline uint32_t queue_pop(int queue_num) {
  uintptr_t queue_addr = QUEUE_BASE + queue_num << 3;
  uint32_t data;
  do {
    data = reg_read32(queue_addr);
  } while(data == 0);
  return data;
}

// pushes some nonzero data to the queue
inline void queue_push(int queue_num, uint32_t data) {
  uintptr_t queue_addr = QUEUE_BASE + queue_num << 3;
  uint8_t slot;
  do {
    slot = reg_read8(queue_addr + 4);
  } while(slot == 0);
  reg_write32(queue_addr, data);
}



#define CHEESE1 0x4000
#define CHEESE2 0x4004


int main(void)
{
  printf("Starting cheese test\n");

  printf("e?: %d\n", reg_read8(CHEESE1));
  printf("e?: %d\n", reg_read8(CHEESE2));

  printf("e0: %d\n", reg_read8(CHEESE1));
  printf("e0: %d\n", reg_read8(CHEESE1));
  printf("e0: %d\n", reg_read8(CHEESE2));
  printf("e0: %d\n", reg_read8(CHEESE2));
  reg_write8(CHEESE1, 0);
  printf("e1: %d\n", reg_read8(CHEESE1));
  printf("e0: %d\n", reg_read8(CHEESE2));
  printf("e0: %d\n", reg_read8(CHEESE1));
  printf("e0: %d\n", reg_read8(CHEESE2));
  reg_write8(CHEESE1, 0);
  reg_write8(CHEESE1, 0);
  printf("e1: %d\n", reg_read8(CHEESE1));
  printf("e0: %d\n", reg_read8(CHEESE1));
  reg_write8(CHEESE2, 0);
  printf("e1: %d\n", reg_read8(CHEESE2));
  printf("e0: %d\n", reg_read8(CHEESE2));


  return 0;
}
