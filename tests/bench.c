#include <unistd.h>
#include <sys/types.h>
#include "mmio.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define LOCK_BASE 0x4000
#define NUM_LOCKS 8

// initialize lock to unlocked state
inline static void lock_init(int lock_num) {
  uintptr_t lock_addr = LOCK_BASE + (lock_num << 2);
  reg_write8(lock_addr, 0);
}

inline static void lock_acquire(int lock_num) {
  uintptr_t lock_addr = LOCK_BASE + (lock_num << 2);
  uint8_t ack;
  do {
    ack = reg_read8(lock_addr);
  } while(ack == 0);
}

inline static void lock_release(int lock_num) {
  uintptr_t lock_addr = LOCK_BASE + (lock_num << 2);
  reg_write8(lock_addr, 0);
}


#define QUEUE_BASE 0x4020
#define QUEUE_DEPTH 4
#define NUM_QUEUES 4

// initialize queue to empty state
inline static void queue_init(int queue_num) {
  uintptr_t queue_addr = QUEUE_BASE + (queue_num << 3);
  for (int a = 0; a < QUEUE_DEPTH; a++) {
    reg_read32(queue_addr);
  }
}

// pops some nonzero data from the queue
inline static uint32_t queue_pop(int queue_num) {
  uintptr_t queue_addr = QUEUE_BASE + (queue_num << 3);
  uint32_t data;
  do {
    data = reg_read32(queue_addr);
  } while(data == 0);
  return data;
}

// pushes some nonzero data to the queue
inline static void queue_push(int queue_num, uint32_t data) {
  uintptr_t queue_addr = QUEUE_BASE + (queue_num << 3);
  uint8_t slot;
  do {
    slot = reg_read8(queue_addr + 4);
  } while(slot == 0);
  reg_write32(queue_addr, data);
}


int main(void)
{
  printf("Starting bench\n");

  queue_init(0);

  pid_t pid = fork();
  printf("Forked %d\n", pid);
  if (pid == -1) {
    printf("Error forking process: %s\n", strerror(errno));
    return -1;
  }

  // Hack of a barrier
  queue_push(pid, 200);
  int other_pid = 1 - pid;
  queue_pop(other_pid);
  printf("Passed barrier\n");

  // Hack of a benchmark
  for (int j = 0; j < 10; j++) {
    for (int i = 0; i < 2; i++) {
        queue_push(0, pid+5);
        uint32_t val = queue_pop(0);
        printf("Val: %d\n", val);
    }
  }

  return 0;
}
