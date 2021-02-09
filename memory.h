#ifndef SNP_MEMORY_H_
#define SNP_MEMORY_H_

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

namespace snp {
  class Memory
  {

  private:
      typedef struct heap_chunk
      {
          int corruption_check;
          size_t data_size;
          int available;
          struct heap_chunk *prev;
          struct heap_chunk *next;
          char data[0]; // array of variable size
      } heap_chunk;

      static heap_chunk *heap_start;
      static heap_chunk *heap_end;
      static pthread_mutex_t mutex;

      static void* createChunk(size_t size);
      static void* findAvailableChunk(size_t size);
      static void splitChunk(heap_chunk *chunk, size_t size);
      static heap_chunk *mergeChunk(heap_chunk *chunk);
      static void checkHeapIntegrity();

  public:
    static void *malloc(size_t size);
    static void free(void *ptr);

    static void *_new(size_t size);
    static void _delete(void *ptr);

    static void printStatistics(const char *title = nullptr);
  };
}


void* operator new(size_t size);

void operator delete(void *address ) noexcept;

void* operator new[] ( size_t size );

void operator delete[] ( void* address ) noexcept;

#endif /* SNP_MEMORY_H_ */
