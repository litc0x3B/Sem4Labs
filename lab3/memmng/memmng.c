#include "memmng.h"

#include <pthread.h>
#include <stddef.h>
#include <sys/mman.h>

Block *g_firstBlock = NULL;
Block *g_lastBlock = NULL;
pthread_mutex_t g_MutexMemMng = PTHREAD_MUTEX_INITIALIZER;

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

void *GetData(Block *block) { return (void *)((char *)block + sizeof(Block)); }

Block *GetBlockFromData(void *data)
{
  return (void *)((char *)data - sizeof(Block));
}

bool IsLastBlockInHeap(Block *block)
{
  return (block->heap->size == block->size + sizeof(Block));
}

Block *TryFindFreeBlock(size_t size)
{
  if (g_firstBlock == NULL)
  {
    return NULL;
  }

  if (g_firstBlock == g_lastBlock)
  {
    if (g_firstBlock->size >= size && g_firstBlock->isFree)
    {
      return g_firstBlock;
    }

    return NULL;
  }

  Block *curBlock1 = g_firstBlock;
  Block *curBlock2 = g_lastBlock;

  do
  {
    if (curBlock1->size >= size && curBlock1->isFree)
    {
      return curBlock1;
    }
    curBlock1 = curBlock1->next;

    if (curBlock2->size >= size && curBlock2->isFree)
    {
      return curBlock2;
    }
    curBlock2 = curBlock2->prev;
  } while (curBlock2->next != curBlock1 && curBlock2 != curBlock1);

  return NULL;
}

Block *SplitBlock(size_t newSize, Block *oldBlock)
{
  if (newSize + sizeof(Block) >= oldBlock->size)
  {
    return oldBlock;
  }

  Block *block1 = oldBlock;

  Block *block2 = (Block *)((char *)GetData(oldBlock) + newSize);
  *block2 = *oldBlock;

  block2->prev = block1;
  block2->size -= newSize + sizeof(Block);
  if (block2->next != NULL)
  {
    block2->next->prev = block2;
  }

  block1->next = block2;
  block1->size = newSize;

  if (oldBlock == g_lastBlock)
  {
    g_lastBlock = block2;
  }

  return block1;
}

Block *MergeBlockWithNext(Block *block)
{
  if (block == NULL || block->next == NULL || block->next->heap != block->heap)
  {
    return block;
  }

  if (block->next == g_lastBlock)
  {
    g_lastBlock = block;
  }

  block->size += block->next->size + sizeof(Block);
  if (block->next->next != NULL)
  {
    block->next->next->prev = block;
  }

  block->next = block->next->next;

  return block;
}

Block *RemoveBlock(Block *block)
{
  if (block == NULL)
  {
    return block;
  }

  Block *nextBlock = block->next;
  Block *prevBlock = block->prev;

  if (prevBlock == NULL && nextBlock == NULL)
  {
    g_firstBlock = NULL;
    g_lastBlock = NULL;
    return block;
  }

  if (nextBlock == NULL)
  {
    prevBlock->next = NULL;
    g_lastBlock = prevBlock;
    return block;
  }

  if (prevBlock == NULL)
  {
    nextBlock->prev = NULL;
    g_firstBlock = nextBlock;
    return block;
  }

  nextBlock->prev = block->prev;
  prevBlock->next = block->next;

  return block;
}

Heap *HeapInit(size_t size, Heap *newHeap)
{
  newHeap->size = size;

  size -= sizeof(Block);
  Block *newBlock = GetFirstBlock(newHeap);
  *newBlock = (Block){.heap = newHeap,
                      .prev = NULL,
                      .next = NULL,
                      .isFree = true,
                      .size = size};

  if (g_firstBlock == NULL)
  {
    g_firstBlock = newBlock;
    g_lastBlock = newBlock;
    return newHeap;
  }

  g_lastBlock->next = newBlock;
  newBlock->prev = g_lastBlock;
  g_lastBlock = newBlock;

  return newHeap;
}

Heap *MapHeap(size_t size)
{
  size = (size + sizeof(Heap) + sizeof(Block) + GetPageSize()) * 2 /
         GetPageSize() * GetPageSize();
  if (size < MIN_HEAP_SIZE)
  {
    size = MIN_HEAP_SIZE;
  }
  Heap *newHeap = mmap(NULL, size, PROT_READ | PROT_WRITE,
                       MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  return HeapInit(size - sizeof(Heap), newHeap);
}

void *MyMalloc(size_t size)
{
  const size_t align = _Alignof(max_align_t);
  size = size + align & ~align;
  pthread_mutex_lock(&g_MutexMemMng);

  Block *resBlock;
  if ((resBlock = TryFindFreeBlock(size)) == NULL)
  {
    resBlock = GetFirstBlock(MapHeap(size));
  }

  resBlock = SplitBlock(size, resBlock);
  resBlock->isFree = false;
  pthread_mutex_unlock(&g_MutexMemMng);

  return GetData(resBlock);
}

void MyFree(void *ptr)
{
  pthread_mutex_lock(&g_MutexMemMng);
  Block *block = GetBlockFromData(ptr);

  if (ptr == NULL)
  {
    return;
  }

  if (block->next != NULL && block->next->isFree)
  {
    block = MergeBlockWithNext(block);
  }

  if (block->prev != NULL && block->prev->isFree)
  {
    block = MergeBlockWithNext(block->prev);
  }

  block->isFree = true;
  if (IsLastBlockInHeap(block))
  {
    RemoveBlock(block);
    munmap(block->heap, block->heap->size);
  }
  pthread_mutex_unlock(&g_MutexMemMng);
}
