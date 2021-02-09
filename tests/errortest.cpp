#include "../memory.h"
#include <cstdio>
#include <cstring>

#define TEST 1

int main()
{
#if TEST == 1
  // TEST 1: Memory corruption
  char *test1 = (char*) snp::Memory::malloc(64);
  snp::Memory::malloc(40);
  snp::Memory::printStatistics();
  snp::Memory::free(test1+20);
  // exit(-1) because test1+20 is not the pointer returned by malloc

#elif TEST == 2
  // TEST 2: Out of memory
  char *test2 = (char*) snp::Memory::malloc(4076);
  snp::Memory::printStatistics();
  snp::Memory::free(test2+1);
  // exit(-1) because test2+1 is not allocated

#elif TEST == 3
  // TEST 3: Double free
  char *test3 = (char*) snp::Memory::malloc(64);
  char *test4 = (char*) snp::Memory::malloc(40);
  snp::Memory::free(test3);
  snp::Memory::free(test3);
  // exit(-1) because test3 is already marked as available=1

#elif TEST == 4
  // TEST 4: Overflow the size_t given to sbrk -> size_t + HEAP_CHUNK_SIZE
  snp::Memory::malloc((size_t)-5);
  // exit(-1) because -5 overflows to 4294967291 => + HEAP_CHUNK_SIZE) > size_t maximum

#elif TEST == 5
  // TEST 5a: Heap overflow
  char *test5a = (char*) snp::Memory::malloc(5);
  char *test5b = (char*) snp::Memory::malloc(5);
  strcpy(test5a, "AAAAAA");
  snp::Memory::printStatistics();
  snp::Memory::free(test5a);
  // exit(-1) because strcpy writes AAAAAA\0 -> overwriting corruption_check

#elif TEST == 6
  // TEST 6: Heap overflow: Same overflow as 5 but with malloc() instead of free()
  char *test6a = (char *) snp::Memory::malloc(5);
  char *test6b = (char *) snp::Memory::malloc(5);
  snp::Memory::free(test6a);
  strcpy(test6a, "AAAAAA");
  snp::Memory::printStatistics();
  test6b = (char *) snp::Memory::malloc(5);
  // exit(-1) because strcpy writes AAAAAA\0 -> overwriting corruption_check

#elif TEST == 7
  // TEST 7: Memory corruption: data_size is manipulated
  // when chunk->next != nullptr
  char *test7a = (char *) snp::Memory::malloc(123);
  char *test7b = (char *) snp::Memory::malloc(123);
  snp::Memory::printStatistics();
  *(size_t*) (test7a-0x10) = 5;
  snp::Memory::printStatistics();
  snp::Memory::free(test7a);
  // exit(-1) because the next chunk does not begin after data+data_size

#elif TEST == 8
  // TEST 8: Memory corruption: data_size is manipulated
  // when chunk->next == nullptr
  // FIXME: This can only be detected if we check that: (chunk->next == nullptr && (sbrk(0) != chunk_end)
  //  but we should not use sbrk(0) more than once
  char *test8 = (char *) snp::Memory::malloc(4076);
  snp::Memory::printStatistics();
  *(size_t*) (test8-0x10) = 5;
  snp::Memory::printStatistics();
  snp::Memory::free(test8);
  // exit(-1) because the next chunk does not begin after data+data_size

#elif TEST == 9
  // TEST 9: Memory corruption: prev pointer is invalid
  char *test9 = (char *) snp::Memory::malloc(4076);
  *(test9-8) = 0x4;
  snp::Memory::printStatistics();
  snp::Memory::free(test9);
  // exit(-1) because 0x4 is < heap_start

#elif TEST == 10
  // TEST 10: Memory corruption: next pointer == current chunk
  char *test10 = (char *) snp::Memory::malloc(4076);
  char *test10_chunk_start = test10-0x14;
  char *test10_chunk_next = test10-0x4;
  *(int*) test10_chunk_next = (int) test10_chunk_start;
  snp::Memory::free(test10);
  // exit(-1) because the fake next chunk does not start where test10 + data_size ends
#endif

  return 0;
}
