// threadtest.c

#include "../mythreads.h"
#include <stdio.h>
#include <stdlib.h>

void *t1(void *arg) {
  int param = *((int *)arg);
  printf("t1 started %d\n", param);

  threadLock(9);

  threadYield();

  int *result = malloc(sizeof(int));
  *result = param + 1;
  printf("added 1! (%d)\n", *result);

  threadYield();

  threadUnlock(9);

  printf("t1: done result=%d\n", *result);
  return result;
}

int main(void) {
  int id1, id2, id3;
  int p1, p2, p3;

  p1 = 1;
  p2 = 2;
  p3 = 3;

  int *result1, *result2, *result3;

  // initialize the threading library. DON'T call this more than once!!!
  threadInit();

  id1 = threadCreate(t1, (void *)&p1);
  printf("created thread 1.\n");

  threadJoin(id1, (void *)&result1);
  printf("joined #1 --> %d.\n", *result1);

  id2 = threadCreate(t1, (void *)&p2);
  printf("created thread 2.\n");

  threadJoin(id2, (void *)&result2);
  printf("joined #2 --> %d.\n", *result2);

  id3 = threadCreate(t1, (void *)&p3);
  printf("created thread 3.\n");

  threadJoin(id3, (void *)&result3);
  printf("joined #3 --> %d.\n", *result3);
}
