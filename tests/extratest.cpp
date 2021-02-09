#include "unistd.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include "../memory.h"

#define PAGESIZEALLOC 1

#if PAGESIZEALLOC == 1
  #define ALLOCATION_SIZE getpagesize()
#else
  #define ALLOCATION_SIZE (allocation + HEAP_CHUNK_SIZE)
#endif
#define HEAP_CHUNK_SIZE 20

int main()
{
  char *heap_start = (char*) sbrk(0);
  //printf("HEAP START: %p\n", heap_start);

  // TEST 1: allocating 1 int should allocate an entire page
  size_t allocation = sizeof(int);
  int *test1 = (int*) snp::Memory::malloc(allocation);
  *test1 = 5;
  char *heap_ptr = (char*) sbrk(0);
  //snp::Memory::printStatistics("TEST 1: AFTER malloc");
  assert (heap_ptr == heap_start + ALLOCATION_SIZE);

  snp::Memory::free(test1);
  //snp::Memory::printStatistics("TEST 1: AFTER free");
  heap_ptr = (char*) sbrk(0);
  assert (heap_ptr == heap_start);

  // TEST 2: malloc for test2b should reuse the test2a chunk
  char *test2a = (char*) snp::Memory::malloc(20);
  char *test2c = (char*) snp::Memory::malloc(30);
  snp::Memory::free(test2a);
  char *test2b = (char*) snp::Memory::malloc(1);
  assert (test2a == test2b);

  snp::Memory::free(test2b);
  snp::Memory::free(test2c);
  //snp::Memory::printStatistics("TEST 2: AFTER free");
  heap_ptr = (char*) sbrk(0);
  assert (heap_ptr == heap_start);

  // TEST 3: malloc(0) should return a pointer that can be freed
  char *test3 = (char*) snp::Memory::malloc(0);
  assert (test3 != nullptr);
  snp::Memory::free(test3);
  //snp::Memory::printStatistics("TEST 3: AFTER free");
  heap_ptr = (char*) sbrk(0);
  assert (heap_ptr == heap_start);

  // TEST 4: malloc for test4c should reuse the test4a chunk
  char *test4a = (char*) snp::Memory::malloc(40);
  char *test4b = (char*) snp::Memory::malloc(32);
  snp::Memory::free(test4a);
  //snp::Memory::printStatistics("TEST 4: AFTER free test4a");
  char *test4c = (char*) snp::Memory::malloc(15);
  //snp::Memory::printStatistics("TEST 4: AFTER malloc(15)");
  assert (test4a == test4c);
  snp::Memory::free(test4b);
  snp::Memory::free(test4c);
  //snp::Memory::printStatistics("TEST 4: AFTER free");
  heap_ptr = (char*) sbrk(0);
  assert (heap_ptr == heap_start);

  // TEST 5: when doing malloc(1) 243 times in total only one page should be called via sbrk()
  // => 147x HEAP_CHUNK_SIZE (20) + 1 byte => 3087 bytes
  int calls = 147;
  char *ptr[calls];
  allocation = sizeof(char);
  for (int i = 0; i < calls; i++)
    ptr[i] = (char*) snp::Memory::malloc(allocation);
  heap_ptr = (char*) sbrk(0);
  //snp::Memory::printStatistics("TEST 5: AFTER malloc");
#if PAGESIZEALLOC == 1
  assert (heap_ptr == heap_start + 1 * ALLOCATION_SIZE);
#else
  assert (heap_ptr == heap_start + calls * ALLOCATION_SIZE);
#endif

  for (int i = 0; i < calls; i++)
    snp::Memory::free(ptr[i]);

  //snp::Memory::printStatistics("TEST 5: AFTER free");
  heap_ptr = (char*) sbrk(0);
  assert (heap_ptr == heap_start);

  // TEST 6: regular allocation to test chunk reuse after free/merge
  char *test6a = (char*) snp::Memory::malloc(400);
  char *test6b = (char*) snp::Memory::malloc(10600);
  //snp::Memory::printStatistics("TEST 6: AFTER malloc");
  snp::Memory::free(test6b);
  //snp::Memory::printStatistics("TEST 6: AFTER free 6b");
  char *test6c = (char*) snp::Memory::malloc(3800);
  //snp::Memory::printStatistics("TEST 6: AFTER malloc 6c");
  snp::Memory::free(test6c);
  snp::Memory::free(test6a);
  //snp::Memory::printStatistics("TEST 6: AFTER free 6c,6a");
  heap_ptr = (char*) sbrk(0);
  assert (heap_ptr == heap_start);

  return 0;
}
