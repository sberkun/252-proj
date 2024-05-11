#include "mmio.h"



// helper

static inline int rdcycle() {
    int cycles;
    asm volatile ("rdcycle %0" : "=r"(cycles));
    return cycles;
}

static inline int mhartid() {
    int temp;
    asm volatile ("csrr %0, mhartid" : "=r"(temp));
    return temp;
}


#define read_csr(reg) __extension__ ({ \
    unsigned long __tmp; \
    __asm__ __volatile__ ("csrr %0, " #reg : "=r"(__tmp)); \
    __tmp; })




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



#define MONITOR_BASE 0x4040

  inline static void monitor_acquire(uintptr_t lock_id, int hartid) {
    uintptr_t lock_addr = MONITOR_BASE + (hartid << 3);
    reg_write32(lock_addr + 4, lock_id);
    uint8_t ack;
    do {
      ack = reg_read8(lock_addr);
    } while(ack == 0);
  }

  inline static void monitor_release(int hartid) {
    uintptr_t lock_addr = MONITOR_BASE + (hartid << 3);
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
