#include <new>
#include "memory.h"

void *snp::Memory::_new(size_t size)
{
  void *p = malloc(size);
  // Return bad_alloc if allocation fails.
  // If size == 0 -> it should still return "a distinct non-null pointer"
  if (!p)
    throw std::bad_alloc();

  return p;
}

void snp::Memory::_delete(void * p)
{ 
  return free(p);
}

void* operator new(size_t size)
{
  return snp::Memory::_new(size);
}

void operator delete(void *address ) noexcept
{
  snp::Memory::_delete(address);
}

void* operator new[] ( size_t size )
{
  return snp::Memory::_new(size);
}

void operator delete[] ( void* address ) noexcept
{
  snp::Memory::_delete(address);
}