#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>


void *MyMalloc(size_t size);
void MyFree(void *ptr);


//internal funcs and defs
typedef struct s_heap {
  size_t size;
} Heap;

typedef struct s_block {
  Heap *heap;
  struct s_block *prev;
  struct s_block *next;
  bool isFree;
  size_t size;
} Block;

Block *GetFirstBlock(Heap *heap);
void *GetData(Block *block);
Block *GetBlockFromData(void *data);
bool IsLastBlockInHeap(Block *block);

Block *TryFindFreeBlock(size_t size);
Block *SplitBlock(size_t newSize, Block *block);
Block *MergeBlockWithNext(Block *block);


Heap *MapHeap(size_t size);


