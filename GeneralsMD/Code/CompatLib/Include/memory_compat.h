#pragma once

#include <stdlib.h>

#ifdef __linux__
#include <malloc.h>
#elif __APPLE__
#include <malloc/malloc.h>
#endif

#define GMEM_FIXED 0

static void *GlobalAlloc(int, size_t size)
{
  return malloc(size);
}

static void GlobalFree(void *ptr)
{
  free(ptr);
}

static size_t GlobalSize(void *ptr)
{
#ifdef __linux__
  return malloc_usable_size(ptr);
#elif defined(__APPLE__)
  return malloc_size(ptr);
#else
  #error "GlobalSize not implemented for this platform"
#endif
}

struct MEMORYSTATUS;