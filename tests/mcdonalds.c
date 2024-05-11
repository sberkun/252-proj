#include "locks_queues.h"




#define NUGGET1 0x4040
#define NUGGET2 0x4048


int main(void)
{
  printf("Starting cheese test\n");

  int x1 = rdcycle();

  int x2 = rdcycle();

  printf("irfj %d %d\n", x1, x2);

  reg_write32(NUGGET1 + 4, 5);
  reg_write32(NUGGET2 + 4, 6);

  printf("e?: %d\n", reg_read8(NUGGET1));
  printf("e?: %d\n", reg_read8(NUGGET2));

  reg_write8(NUGGET1, 0);
  reg_write8(NUGGET2, 0);

  printf("e?: %d\n", reg_read8(NUGGET1));
  printf("e?: %d\n", reg_read8(NUGGET2));

  reg_write8(NUGGET1, 0);
  reg_write8(NUGGET2, 0);

  reg_write32(NUGGET2 + 4, 5);
  
  printf("e0: %d\n", reg_read8(NUGGET1));
  printf("e0: %d\n", reg_read8(NUGGET2));

  printf("e0: %d\n", reg_read32(NUGGET1 + 4));
  printf("e0: %d\n", reg_read32(NUGGET2 + 4));

  printf("e0: %d\n", reg_read8(NUGGET1));
  printf("e0: %d\n", reg_read8(NUGGET2));

  printf("irfj %d\n", rdcycle());

  return 0;
}
