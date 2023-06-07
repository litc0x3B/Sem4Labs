#include "memmng.h"

#include <pthread.h>
#include <stddef.h>
#include <sys/mman.h>


Block *g_firstBlock = NULL;
pthread_mutex_t g_MutexMalloc = PTHREAD_MUTEX_INITIALIZER;

long GetPageSize()
{
  static long pageSize = 0;

  if (pageSize == 0)
  {
    pageSize = sysconf(_SC_PAGESIZE);
  }

  return pageSize;
}

#define MIN_HEAP_SIZE 5 * GetPageSize()

Block *GetFirstBlock(Heap *heap)
{
  return (Block *)((char *)heap + sizeof(Heap));
}

void *GetData(Block *block)
{
  return (void *)((char *)block + sizeof(Block));
}

Block *TryFindFreeBlock(size_t size)
{
  Block *curBlock = g_firstBlock;
  while (curBlock != NULL)
  {
    if (curBlock->size >= size && curBlock->isFree)
    {
      return curBlock;
    }
    curBlock = curBlock->next;
  }

  return NULL;
}


Block *SplitBlock(size_t newSize, Block *block)
{

  if (newSize + sizeof(Block) >= block->size)
  {
    return block;
  }

  Block *block1 = block;
  Block *block2 = (Block *)((char *)GetData(block) + newSize);

  *block2 = *block;
  block2->size = block->size - sizeof(Block) - newSize;
  block2->prev = block1;

  block1->size = newSize;
  block1->next = block2;

  return block1;
}

Heap *HeapInit(size_t size, Heap *newHeap)
{
  newHeap->size = size;

  //creating block of size of whole heap
  size -= sizeof(Block);
  Block *newBlock = GetFirstBlock(newHeap);
  *newBlock = (Block){.heap = newHeap, .prev = NULL, .next = NULL, .isFree = true, .size = size};

  if (g_firstBlock == NULL)
  {
    g_firstBlock = newBlock;
    return newHeap;
  }

  Block *curBlock = g_firstBlock;
  while(curBlock->next != NULL)
  {
    curBlock = curBlock->next;
  }

  curBlock->next = newBlock;
  newBlock->prev = curBlock;

  return newHeap;
}

Heap *MapHeap(size_t size) 
{
  //allocating new heap
  size = (size + sizeof(Heap) + sizeof(Block) + GetPageSize()) * 2 / GetPageSize() * GetPageSize();
  if (size < MIN_HEAP_SIZE)
  {
    size = MIN_HEAP_SIZE;
  }
  Heap *newHeap = mmap(NULL,  size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  return HeapInit(size - sizeof(Heap), newHeap);
}

void *MyMalloc(size_t size)
{
  const size_t align = _Alignof(max_align_t);
  size = size + align & ~align;
  pthread_mutex_lock(&g_MutexMalloc);

  Block *resBlock;
  if ((resBlock = TryFindFreeBlock(size)) == NULL)
  {
    resBlock = GetFirstBlock(MapHeap(size));
  }

  resBlock = SplitBlock(size, resBlock);
  resBlock->isFree = false;
  pthread_mutex_unlock(&g_MutexMalloc);

  return GetData(resBlock);
}