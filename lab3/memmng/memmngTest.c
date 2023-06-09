#include <stdio.h>

#include "memmng.h"

int main()
{
  printf("oldBlock: %lu\n", GetFirstBlock(MapHeap(32))->size);
  Block *block = SplitBlock(16, TryFindFreeBlock(32));
  printf("block: %lu, block->next: %lu, block->next->prev: %lu\n", block->size,
         block->next->size, block->next->prev->size);
  printf("sizeof(Block): %lu\n", sizeof(Block));
  block->isFree = false;
  printf("free block: %lu\n", TryFindFreeBlock(1)->size);

  block = MergeBlockWithNext(block);
  printf("merged block: %lu, block->next: %p\n", block->size, block->next);
  printf("merged block: %p, GetBlock(GetData(block)): %p\n", block,
         GetBlockFromData(GetData(block)));

  printf("merged block heap: %p\n", block->heap);
  printf("*GetData(merged block) = 3\n");
  *(int *)GetData(block) = 3;
  printf("merged block heap: %p\n", block->heap);

  const int length = 1000;
  const int size = sizeof(long long);
  long long *arr[length];

  for (int i = 0; i < length; i++)
  {
    arr[i] = MyMalloc(size);
    *arr[i] = i;
  }

  for (int i = 0; i < length; i++)
  {
    printf("%llu\n", *arr[i]);
    MyFree(arr[i]);
  }

  // for (int i = 0; i < size / 2; i++)
  // {
  //     arr[i] = MyMalloc(sizeof(long long));
  //     *arr[i] = i;
  // }

  // for (int i = 0; i < size; i++)
  // {
  //     printf("%llu\n", *arr[i]);
  //     MyFree(arr[i]);
  // }

  printf("\n");
}
