#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../mythreads.h"

void *t1 (void *arg) {
	assert(!interruptsAreDisabled);

	threadLock(2);

	assert(!interruptsAreDisabled);

	int param = *((int*)arg);
	printf("t1 started %d\n",param);

	assert(!interruptsAreDisabled);

	int *result = malloc(sizeof(int));
	*result = param * 5;
	printf ("timed by 5! (%d)\n",*result);

	assert(!interruptsAreDisabled);

	threadYield();

	assert(!interruptsAreDisabled);

	threadUnlock(2);

	assert(!interruptsAreDisabled);

	printf("t1: done result=%d\n",*result);
	return  result;
}

void *t2 (void *arg) {
	assert(!interruptsAreDisabled);

	threadLock(2);

	assert(!interruptsAreDisabled);

	int param = *((int*)arg);
	printf("t2 started %d\n",param);

	assert(!interruptsAreDisabled);

	int *result = malloc(sizeof(int));
	*result = param + 5;
	printf ("added 5! (%d)\n",*result);

	assert(!interruptsAreDisabled);

	threadSignal(1, 1);

	assert(!interruptsAreDisabled);

	threadUnlock(2);

	assert(!interruptsAreDisabled);

	int id1, id2;

	int p1 = 20;

	int *result1, *result2;

	assert(!interruptsAreDisabled);

	id1 = threadCreate(t1,(void*)&p1);
	printf("created thread 1.\n");

	assert(!interruptsAreDisabled);

	id2 = threadCreate(t1,(void*)&p1);
	printf("created thread 2.\n");

	assert(!interruptsAreDisabled);

	threadJoin(id1, (void*)&result1);
	printf("joined #1 --> %d.\n",*result1);

	assert(!interruptsAreDisabled);

	threadJoin(id2, (void*)&result2);
	printf("joined #2 --> %d.\n",*result2);
	printf("t2: done result=%d\n",*result);

	assert(!interruptsAreDisabled);

	return result;
}

int main(void) {
	int id1, id2;

	int p1 = 20;

	int *result1, *result2;

	threadInit();

	assert(!interruptsAreDisabled);

	id1 = threadCreate(t1,(void*)&p1);
	printf("created thread 1.\n");

	assert(!interruptsAreDisabled);

	id2 = threadCreate(t2,(void*)&p1);
	printf("created thread 2.\n");

	assert(!interruptsAreDisabled);

	threadJoin(id1, (void*)&result1);
	printf("joined #1 --> %d.\n",*result1);

	assert(!interruptsAreDisabled);

	threadJoin(id2, (void*)&result2);
	printf("joined #2 --> %d.\n",*result2);
	assert(!interruptsAreDisabled);

}
