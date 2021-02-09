#include <cstdio>
#include "memory.h"

#define HEAP_CHUNK_SIZE sizeof(heap_chunk)
#define MEMCHECK_NUMBER 1073197

#define PAGESIZEALLOC 1

snp::Memory::heap_chunk* snp::Memory::heap_start = nullptr;
snp::Memory::heap_chunk* snp::Memory::heap_end = nullptr;
pthread_mutex_t snp::Memory::mutex = PTHREAD_MUTEX_INITIALIZER;

void *snp::Memory::malloc(size_t size){
  void *ptr = nullptr;

  pthread_mutex_lock(&mutex);

  checkHeapIntegrity();

  // If the heap is initialized, try to reuse a free chunk
  if (heap_start)
    ptr = findAvailableChunk(size);

  // If there is no chunk available or it is the first allocation.
  // Returns a nullptr if sbrk fails
  if (ptr == nullptr)
    ptr = createChunk(size);

  pthread_mutex_unlock(&mutex);

  return ptr;
}

void snp::Memory::free(void *ptr)
{
  // If ptr is NULL, no operation should be performed (POSIX)
  if (!ptr)
    return;

  pthread_mutex_lock(&mutex);

  //printStatistics("BEFORE free()");

  checkHeapIntegrity();

  // Get the heap chunk holding ptr
  auto *chunk = (heap_chunk*) ((char*) ptr - HEAP_CHUNK_SIZE);

  // Out of memory check -> prevent that someone frees something that is not allocated
  if (heap_start == nullptr || heap_end == nullptr || chunk < heap_start || chunk > heap_end)
    exit(-1);

  // Memory corruption check -> prevent that someone does free(ptr+5)
  // The given ptr seems valid if we find the special number
  if (chunk->corruption_check != MEMCHECK_NUMBER)
    exit(-1);

  // Double free check:
  // free is not possible if the chunk is already marked as available
  if (chunk->available)
    exit(-1);

  chunk->available = 1;

  // Merge with the previous and next chunk if they are marked available
  chunk = mergeChunk(chunk);

  // Try to reduce the program break
#if PAGESIZEALLOC == 1
  if (chunk == heap_end && chunk != heap_start) { // -> there is still > 1 chunks overall
    // We want to reduce the data size but not the struct, e.g.
    // if 12300 would be the entire allocation size -> 12300 - 3*4096 = 12 bytes
    // This will lead to corruption since the header needs 20 bytes
    int pagesize = getpagesize() - HEAP_CHUNK_SIZE;

    // Decrement in multiples of page size, e.g.
    // 3964  <= 4076 -> don't do anything
    // 13000 -> 13000 - 3 * 4076 = 772
    // 12228 -> 12228 - 3 * 4076 = 0 -> 4076
    if (chunk->data_size > (size_t) pagesize) {
      size_t pagesize_multiple = chunk->data_size / pagesize; // int div always does down round
      size_t data_size_to_subtract = pagesize_multiple * pagesize;

      chunk->data_size -= data_size_to_subtract;

      // On error, (void *) -1 is returned, and errno is set to ENOMEM
      if ((int) sbrk(-data_size_to_subtract) == -1)
        exit(-1);
    }
  }
  else
#endif
  if (chunk == heap_end)
  {
    //printStatistics("REDUCE sbrk");

    // If there is no more previous chunk, we are just freeing heap_start -> set it null
    if (chunk->prev == nullptr)
      heap_start = nullptr;
    else
      chunk->prev->next = nullptr;

    // As we are eliminating this chunk, the new heap end is now the previous chunk
    heap_end = chunk->prev;

    // On error, (void *) -1 is returned, and errno is set to ENOMEM
    if ((int) sbrk(-(HEAP_CHUNK_SIZE + chunk->data_size)) == -1)
      exit(-1);
  }

  //printStatistics("AFTER free()");

  pthread_mutex_unlock(&mutex);
}

void* snp::Memory::createChunk(size_t size)
{
  // Prevent that the sum of HEAP_CHUNK_SIZE + size overflows
  auto size_t_max = (size_t)-1;
  if (size > (size_t_max-HEAP_CHUNK_SIZE))
    exit(-1);

  size_t allocation_size = HEAP_CHUNK_SIZE + size;

#if PAGESIZEALLOC == 1
  // If a previous chunk was freed and is marked available at the heap end, let's reuse it.
  // This also means allocating only the missing size now and then merging the old and the new chunk
  // in order to get a chunk with the full size
  if (heap_end != nullptr && heap_end->available)
    allocation_size -= heap_end->data_size;

  // Always allocate in multiples of memory pages (= 4096 bytes)
  int pagesize = getpagesize();

  // If allocation_size is not already a multiple of page size,
  // round up to the next page size
  if (allocation_size % pagesize != 0)
    allocation_size += pagesize - (allocation_size % pagesize);
#endif

  auto *chunk = (heap_chunk *) sbrk(allocation_size);

  // On error, (void *) -1 is returned, and errno is set to ENOMEM
  if ((int) chunk == -1)
    return nullptr;

  chunk->corruption_check = MEMCHECK_NUMBER;
#if PAGESIZEALLOC == 1
  chunk->data_size = allocation_size - HEAP_CHUNK_SIZE;
#else
  chunk->data_size = size;
#endif
  chunk->available = 0;
  chunk->prev = heap_end; // null for the first chunk, otherwise the heap end until now
  chunk->next = NULL;

  // First allocation -> we have a new heap start
  if (heap_start == nullptr) {
    heap_start = chunk;
  }

  // Not the first allocation -> append the new memory chunk to the chunk currently being the heap end
  if (heap_end != nullptr) heap_end->next = chunk;

  // The newly allocated mem chunk is now the new heap end
  heap_end = chunk;

  // Merge with a previous chunk if available
  if (heap_end->prev != nullptr && heap_end->prev->available)
  {
    //printStatistics("BEFORE malloc merge");
    chunk = mergeChunk(chunk);
    chunk->available = 0;
    //printStatistics("AFTER malloc merge");
  }

  // If we have allocated more than needed, resize the current chunk to the actually
  // requested size and move the remaining bytes to a new chunk for potential later use
  if (chunk->data_size > size)
    splitChunk(chunk, size);

  //printStatistics("AFTER createChunk()");

  return chunk->data;
}

void* snp::Memory::findAvailableChunk(size_t size)
{
  heap_chunk *chunk = heap_start;

  while (chunk != nullptr)
  {
    if (chunk->available && (chunk->data_size >= size))
    {
      splitChunk(chunk, size);
      chunk->available = 0;
      return chunk->data;
    }

    chunk = chunk->next;
  }

  return nullptr;
}

void snp::Memory::splitChunk(heap_chunk *chunk, size_t size)
{
  // Building a new chunk only makes sense if there is still space
  // left for some data to store, so > than just HEAP_CHUNK_SIZE
  if ((chunk->data_size - size) <= HEAP_CHUNK_SIZE) {
    return;
  }

  auto *new_chunk = (heap_chunk *) (chunk->data + size);

  new_chunk->corruption_check = MEMCHECK_NUMBER;
  new_chunk->data_size = chunk->data_size - size - HEAP_CHUNK_SIZE;
  new_chunk->available = 1;

  // Insert the new chunk between the previous and the next one
  new_chunk->prev = chunk;
  new_chunk->next = chunk->next;

  // Also update the mappings of our neighbors
  if (chunk->next != nullptr)
    chunk->next->prev = new_chunk;

  chunk->next = new_chunk;

  // If the chunk that we just have split was the last one, the new one is now the heap end
  // e.g., if this was the first malloc ever and we have allocated more than needed -> heap_end == chunk
  if (chunk == heap_end)
    heap_end = new_chunk;

  // The old chunk is now resized
  chunk->data_size = size;
}

snp::Memory::heap_chunk* snp::Memory::mergeChunk(heap_chunk *chunk)
{
  heap_chunk *prev = chunk->prev;
  heap_chunk *next = chunk->next;

  // Merge NEXT heap CHUNK INTO CURRENT one if it is unused
  if (next != nullptr && next->available)
  {
    // The data size of the current chunk increases by the entire size of the next chunk
    // FIXME: chunk->data_size will overflow if the resulting sum is > (size_t)-1
    chunk->data_size += HEAP_CHUNK_SIZE + next->data_size;

    // Also update the mappings of our neighbors
    chunk->next = next->next;
    if (chunk->next != nullptr)
      chunk->next->prev = chunk;

    // If the chunk to merge is the last chunk of the heap,
    // the current one becomes the new heap end
    if (next == heap_end)
      heap_end = chunk;
  }

  // Merge CURRENT heap CHUNK INTO PREVIOUS one if it is unused
  if (prev != nullptr && prev->available)
  {
    // Go back one chunk by making the current heap chunk the next one
    // and the previous one the new current
    next = chunk;
    chunk = prev;

    // The data size of the previous chunk increases by the entire size of this chunk
    // FIXME: chunk->data_size will overflow if the resulting sum is > (size_t)-1
    chunk->data_size += HEAP_CHUNK_SIZE + next->data_size;

    // Also update the mappings of our neighbors
    chunk->next = next->next;
    if (chunk->next != nullptr)
      chunk->next->prev = chunk;

    // If the chunk to merge is the last chunk of the heap,
    // the previous one becomes the new heap end
    if (next == heap_end)
      heap_end = chunk;
  }

  return chunk;
}

void snp::Memory::checkHeapIntegrity()
{
  heap_chunk *chunk = heap_start;

  while (chunk != nullptr)
  {
    // Heap overflow: Detect when MEMCHECK_NUMBER was overwritten
    if (chunk->corruption_check != MEMCHECK_NUMBER)
      exit(-1);

    // == Prevent manipulation of data_size ==

    // 1) Ensure that the next chunk starts immediately after the current one ends
    char *chunk_end = chunk->data + chunk->data_size; // == potential next chunk start
    if (chunk->next != nullptr && (char*) chunk->next != chunk_end)
      exit(-1);

    // 2) If there is no next chunk, ensure there is not more allocated than what we have requested
    // FIXME: This would actually be good but we should not use sbrk(0) more than once
    //  Probably keep the real heap end in an extra variable instead?
    /*if (chunk->next == nullptr && (sbrk(0) != chunk_end))
      exit(-1);*/

    // == Prevent manipulation of prev or next pointer ==

    // 1) prev ptr has to be located between heap_start and the current chunk
    if (chunk->prev != nullptr && (chunk->prev < heap_start || chunk->prev > chunk))
      exit(-1);

    // 2) next ptr has to be located between the current chunk and heap_end
    if (chunk->next != nullptr && (chunk->next < chunk || chunk->next > heap_end))
      exit(-1);

    chunk = chunk->next;
  }
}

void snp::Memory::printStatistics(const char *title)
{
  printf("================\n");
  if (title)
    printf("STATUS    : %s\n", title);
  printf("HEAP START: %p\n", heap_start);
  printf("HEAP END  : %p\n", heap_end);
  printf("sbrk      : %p\n", sbrk(0));

  heap_chunk *chunk = heap_start;
  while (chunk != nullptr)
  {
    printf("------------\n");
    printf("%p: corruption_check: %d\n", chunk, chunk->corruption_check);
    printf("%p: data size: %zu\n", chunk, chunk->data_size);
    printf("%p: available: %d\n", chunk, chunk->available);
    printf("%p: prev: %p\n", chunk, chunk->prev);
    printf("%p: next: %p\n", chunk, chunk->next);
    printf("%p: data: %p\n", chunk, chunk->data);

    chunk = chunk->next;
  }

  printf("================\n\n");
}
