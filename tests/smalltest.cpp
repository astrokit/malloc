#include "../memory.h"
#include "unistd.h"
#include "stdint.h"

int main(void)
{
  int *some = (int*) snp::Memory::malloc(sizeof(int));
  *some = 5;
  snp::Memory::free(some);
  return 0;
}