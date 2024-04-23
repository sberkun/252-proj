#include "mmio.h"

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
