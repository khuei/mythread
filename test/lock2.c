#include <stdio.h>
#include <stdlib.h>
#include "../mythreads.h"

int x = 0;

void* t2 (void *arg) {
    if (interruptsAreDisabled)
        puts("interrupt are disabled");

    printf("Got the value %d\n",*((int*)arg));
    return NULL;
}

void *t1 (void *arg)
{
    if (interruptsAreDisabled)
        puts("interrupt are disabled");

    int param = *((int*)arg);
    printf("t1 started %d\n",param);

    if (interruptsAreDisabled)
        puts("interrupt are disabled");

    // Simple yield
    threadYield();

    if (interruptsAreDisabled)
        puts("interrupt are disabled");

    // Simple Lock
    threadLock(3);

    if (interruptsAreDisabled)
        puts("interrupt are disabled");

    // This signal is important for the second thread...
    threadSignal(3,1);

    if (interruptsAreDisabled)
        puts("interrupt are disabled");

    int* result = malloc(sizeof(int));
    *result = param + 1;
    printf ("added 1! (%d)\n",*result);

    if (interruptsAreDisabled)
        puts("interrupt are disabled");

    // Simple Wait
    threadWait(3,1);

    if (interruptsAreDisabled)
        puts("interrupt are disabled");

    // End of critical section
    threadUnlock(3);

    if (interruptsAreDisabled)
        puts("interrupt are disabled");

    // Signal the second thread
    threadSignal(3,1);

    if (interruptsAreDisabled)
        puts("interrupt are disabled");

    // Simple yield
    threadYield();

    if (interruptsAreDisabled)
        puts("interrupt are disabled");

    printf("t1: done result=%d\n",*result);

    // Thread from a thread
    if(x++ == 0) {
        int p3 = 45;
        int id3 = threadCreate(t2,(void*)&p3);

        if (interruptsAreDisabled)
            puts("interrupt are disabled");

        threadJoin(id3, NULL);

        if (interruptsAreDisabled)
            puts("interrupt are disabled");
    }

    return result;
}

int main(void)
{
    int id1, id2;
    int p1;
    int p2;

    p1 = 23;
    p2 = 2;

    int *result1, *result2;

    //initialize the threading library. DON'T call this more than once!!!
    threadInit();

    if (interruptsAreDisabled)
        puts("interrupt are disabled");

    id1 = threadCreate(t1,(void*)&p1);
    printf("created thread 1.\n");

    if (interruptsAreDisabled)
        puts("interrupt are disabled");

    id2 = threadCreate(t1,(void*)&p2);
    printf("created thread 2.\n");

    if (interruptsAreDisabled)
        puts("interrupt are disabled");

    printf("Let's try and join!\n");
    threadJoin(id1, (void*)&result1);
    printf("joined #1 --> %d.\n",*result1);

    if (interruptsAreDisabled)
        puts("interrupt are disabled");

    threadJoin(id2, (void*)&result2);
    printf("joined #2 --> %d.\n",*result2);

    if (interruptsAreDisabled)
        puts("interrupt are disabled");
}
