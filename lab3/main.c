#include <limits.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>

#include "memmng/memmng.h"

// you can change theese consts however you want
#define THREADS_PER_FUNC 20  // THREADS / 3
#define OUTPUT_FILE_NAME "out.txt"
const size_t BLOCK_SIZES[] = {16, 1024, 1024 * 1024};
//-----------------------------------------------------------------

#define BLOCKS_PER_THREAD sizeof(BLOCK_SIZES) / sizeof(size_t)

FILE *g_outputFile;
unsigned char *g_blocks[THREADS_PER_FUNC][BLOCKS_PER_THREAD];

void *AllocThread(void *arg)
{
  int threadGroup = *(int *)arg;

  for (int blockNum = 0; blockNum < BLOCKS_PER_THREAD; blockNum++)
  {
    printf("thread group %d: is allocaing block of %lu bytes\n", threadGroup,
           (unsigned long)BLOCK_SIZES[blockNum]);
    g_blocks[threadGroup][blockNum] =
        MyMalloc(sizeof(unsigned char) * BLOCK_SIZES[blockNum]);
    printf("thread group %d: allocated block of %lu bytes at the addr %p\n",
           threadGroup, (unsigned long)BLOCK_SIZES[blockNum],
           g_blocks[threadGroup][blockNum]);
  }

  return NULL;
}

void *FillThread(void *arg)
{
  int threadGroup = *(int *)arg;

  printf("thread group %d: filling\n", threadGroup);
  for (int blockNum = 0; blockNum < BLOCKS_PER_THREAD; blockNum++)
  {
    for (int i = 0; i < BLOCK_SIZES[blockNum]; i++)
    {
      g_blocks[threadGroup][blockNum][i] = threadGroup;
    }
  }

  return NULL;
}

void *WriteAndFreeThread(void *arg)
{
  int threadGroup = *(int *)arg;
  printf("thread group %d: waiting before start writing to file\n",
         threadGroup);
  flockfile(g_outputFile);
  printf("thread group %d: writing to file\n", threadGroup);

  for (int blockNum = 0; blockNum < BLOCKS_PER_THREAD; blockNum++)
  {
    fprintf(g_outputFile, "thread group %d, address %p:", threadGroup,
            g_blocks[threadGroup][blockNum]);

    for (int i = 0; i < BLOCK_SIZES[blockNum]; i++)
    {
      fprintf(g_outputFile, " %d", (int)g_blocks[threadGroup][blockNum][i]);
    }

    MyFree(g_blocks[threadGroup][blockNum]);
    fprintf(g_outputFile, "\n");
  }

  funlockfile(g_outputFile);
  return NULL;
}

void *CreateThreadsThread(void *arg)
{
  pthread_t newThread;

  pthread_create(&newThread, NULL, AllocThread, arg);
  pthread_join(newThread, NULL);

  pthread_create(&newThread, NULL, FillThread, arg);
  pthread_join(newThread, NULL);

  pthread_create(&newThread, NULL, WriteAndFreeThread, arg);
  pthread_join(newThread, NULL);

  printf("thread group %d: done!\n", *(int *)arg);
  MyFree(arg);
  return NULL;
}

int main()
{
  _Static_assert(THREADS_PER_FUNC <= UCHAR_MAX + 1,
                 "THREADS_PER_FUNC should be less or equal than UCHAR_MAX + 1");
  g_outputFile = fopen(OUTPUT_FILE_NAME, "w");

  pthread_t threads[THREADS_PER_FUNC];
  for (int threadGroup = 0; threadGroup < THREADS_PER_FUNC; threadGroup++)
  {
    int *threadGroupArg = MyMalloc(sizeof(int));
    *threadGroupArg = threadGroup;
    pthread_create(&threads[threadGroup], NULL, CreateThreadsThread,
                   (void *)threadGroupArg);
  }

  for (int threadGroup = 0; threadGroup < THREADS_PER_FUNC; threadGroup++)
  {
    pthread_join(threads[threadGroup], NULL);
  }

  fclose(g_outputFile);

  return 0;
}
