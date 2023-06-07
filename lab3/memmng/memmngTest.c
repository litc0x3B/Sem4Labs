#include <stdio.h>
#include "memmng.h"

int main()
{
    printf("oldBlock: %lu\n", GetFirstBlock(MapHeap(32))->size);
    Block* block = SplitBlock(16, TryFindFreeBlock(32));
    printf
    (
        "block: %lu, block->next: %lu, block->next->prev: %lu\n", 
        block->size, 
        block->next->size, 
        block->next->prev->size
    );
    printf("sizeof(Block): %lu\n", sizeof(Block));
    block->isFree = false;
    printf("freeBlock: %lu\n", TryFindFreeBlock(1)->size);


    const int size = 10;
    long long *arr[size];

    for (int i = 0; i < size; i++)
    {
        arr[i] = MyMalloc(sizeof(long long));
        *arr[i] = i;
    }

    for (int i = 0; i < size; i++)
    {
        printf("%llu ", *arr[i]);
    }
    printf("\n");
}