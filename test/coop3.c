#include "../mythreads.h"
#include <stdio.h>
#include <stdlib.h>

void* t1(void*);

int x;

int main() {
    x = 0;
    threadInit();

    int id[100000];

    int count = 1;
    for (int i = 0; i < 100000; i++) {
        id[i] = threadCreate(t1, &count);
    }

    int *res = malloc(sizeof(int));
    for (int i = 0; i < 100000; i++) {
        threadJoin(id[i], (void**)&res);
    }

    printf("value of x is %d\n", x);

    return 0;
}

void* t1(void *arg) {
    int *value = (int*)arg;

    for (int i = 0; i < 10; i++) {
        threadLock(0);
        threadYield();
//        threadYield();
//        threadYield();
//        threadYield();
//        threadYield();
//        threadYield();
//        threadYield();
//        threadYield();
//        threadYield();
//        threadYield();
//        threadYield();
//        threadYield();
//        threadYield();
//        threadYield();
//        threadYield();
//        threadYield();
      int temp = x;
        threadYield();
      threadYield();
//        threadYield();
//        threadYield();
//        threadYield();
      x = temp + *value;
      threadYield();
//        threadYield();
//        threadYield();
//        threadYield();
//        threadYield();
//        threadYield();
//        threadYield();
        threadUnlock(0);

    }

    return arg;
}
