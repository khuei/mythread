#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <ucontext.h>
#include <signal.h>

#include "mythreads.h"

#define NUM_CONDITIONS (CONDITIONS_PER_LOCK + 1)
#define SUSPEND_SLOT CONDITIONS_PER_LOCK

enum THREAD_STATE {
	RUNNING,
	FINISHED,
	DEAD,
};

int *threadState;
int threadStateLen;

typedef struct Thread {
	int id;
	void *result;
	ucontext_t *context;
	struct Thread *next;
} Thread;

int threadNum = 0;

typedef struct Queue {
	Thread *head;
	Thread *tail;
} Queue;

Queue *running_queue = NULL;
Queue **join_queue = NULL;
Queue *waiting_queue[NUM_LOCKS][NUM_CONDITIONS] = {NULL};

static Queue *queueInit(void);
static void enqueueHead(Queue *, Thread *);
static void enqueue(Queue *, Thread *);
static Thread *dequeue(Queue *queue);
static bool queueIsEmpty(Queue *);

int interruptsAreDisabled = 0;

sigset_t signal_set;

static void interruptDisable() {
	// Block all signals
	sigemptyset(&signal_set);
	sigfillset(&signal_set); // Block all signals
	sigprocmask(SIG_BLOCK, &signal_set, NULL);
}

static void interruptEnable() {
	// Unblock all signals
	sigprocmask(SIG_UNBLOCK, &signal_set, NULL);
}

int mutex_locks[NUM_LOCKS] = {0};

void threadClean(void);

Queue *queueInit(void) {
	Queue *queue = (Queue *)malloc(sizeof(Queue));
	queue->head = queue->tail = NULL;
	
	return queue;
}

void enqueueHead(Queue *queue, Thread *node) {
	if (queue->head == NULL) {
		queue->head = queue->tail = node;
		node->next = NULL;
	} else {
		node->next = queue->head;
		queue->head = node;
	}
}

void enqueue(Queue *queue, Thread *node) {
	if (queue->head == NULL) {
		queue->head = queue->tail = node;
		queue->tail->next = NULL;
	} else {
		queue->tail->next = node;
		queue->tail = node;
		queue->tail->next = NULL;
	}
}

Thread *dequeue(Queue *queue) {
	if (queue == NULL || queue->head == NULL)
		return NULL;

	Thread *removeThread = queue->head;

	if (queue->head == queue->tail) {
		queue->head = queue->tail = NULL;
		return removeThread;
	}

	queue->head = queue->head->next;

	return removeThread;
}

bool queueIsEmpty(Queue *queue) {
	return (queue != NULL && queue->head == NULL && queue->tail == NULL);
}

void threadInit() {
	interruptDisable();

	threadStateLen = 1;
	threadState = (int *)malloc(sizeof(int) * threadStateLen);

	running_queue = queueInit();

	for (int n = 0; n < NUM_LOCKS; ++n) {
		for (int c = 0; c < NUM_CONDITIONS; ++c) {
			waiting_queue[n][c] = queueInit();
		}
	}

	join_queue = (Queue **)malloc(sizeof(Queue *) * threadStateLen);
	for (int n = 0; n < threadStateLen; ++n)
		join_queue[n] = queueInit();

	Thread *main = (Thread *)malloc(sizeof(Thread));
	main->id = threadNum++;
	main->context = (ucontext_t *)malloc(sizeof(ucontext_t));
	getcontext(main->context);
	main->next = NULL;
	threadState[main->id] = RUNNING;

	enqueue(running_queue, main);

	for (int l = 0; l < NUM_LOCKS; ++l)
		mutex_locks[l] = 0;

	atexit(threadClean);

	interruptEnable();
}

void getResult(thFuncPtr funcPtr,void *argPtr){
	interruptEnable();
	threadExit(funcPtr(argPtr));
}

int threadCreate(thFuncPtr funcPtr, void *argPtr) {
	interruptDisable();

	Thread *newThread = (Thread *)malloc(sizeof(Thread));
	newThread->id = threadNum;
	newThread->result = NULL;
	newThread->next = NULL;

	newThread->context = (ucontext_t *)malloc(sizeof(ucontext_t));
	getcontext(newThread->context);
	newThread->context->uc_stack.ss_sp = malloc(STACK_SIZE);
	newThread->context->uc_stack.ss_size = STACK_SIZE;
	newThread->context->uc_stack.ss_flags = 0;
	makecontext(newThread->context, (void (*)(void))getResult, 2, funcPtr, argPtr);

	Thread *currentThread = dequeue(running_queue);
	enqueue(running_queue, currentThread);
	enqueueHead(running_queue, newThread);

	if (threadStateLen == threadNum) {
		int newThreadStateLen;
		if (threadNum <= 50000)
			newThreadStateLen = threadStateLen * 2;
		else
			newThreadStateLen = threadStateLen + 1;

		threadState = realloc(threadState, sizeof(int) *
		                              newThreadStateLen);
		join_queue = realloc(join_queue, sizeof(Queue *) *
		                               newThreadStateLen);

		for (int i = threadStateLen; i < newThreadStateLen; ++i)
			join_queue[i] = queueInit();

		threadStateLen = newThreadStateLen;
	}

	threadState[newThread->id] = RUNNING;

	swapcontext(currentThread->context, newThread->context);

	interruptEnable();

	return threadNum++;
}

void threadYield() {
	interruptDisable();

	Thread *yieldThread = dequeue(running_queue);
	enqueue(running_queue, yieldThread);
	swapcontext(yieldThread->context, running_queue->head->context);

	interruptEnable();
}

void threadJoin(int id, void **result) {
	interruptDisable();

	if (id > threadNum || id >= threadStateLen || threadState[id] == DEAD) {
		interruptEnable();
		return;
	}

	while (threadState[id] != FINISHED) {
		Thread *currentThread = dequeue(running_queue);
		enqueue(join_queue[id], currentThread);
		swapcontext(currentThread->context, running_queue->head->context);
	}

	Thread *targetThread = dequeue(join_queue[id]);

	if (result != NULL)
		*result = targetThread->result;

	threadState[id] = DEAD;

	free(targetThread);
	targetThread = NULL;
	free(join_queue[id]);
	join_queue[id] = NULL;

	interruptEnable();
}

void threadExit(void *result) {
	interruptDisable();

	Thread *finishedThread = dequeue(running_queue);

	threadState[finishedThread->id] = FINISHED;

	free(finishedThread->context->uc_stack.ss_sp);
	finishedThread->context->uc_stack.ss_size = 0;
	free(finishedThread->context);
	finishedThread->context = NULL;
	finishedThread->result = result;

	while (!queueIsEmpty(join_queue[finishedThread->id]))
		enqueueHead(running_queue, dequeue(join_queue[finishedThread->id]));

	enqueue(join_queue[finishedThread->id], finishedThread);

	if (running_queue->head != NULL)
		setcontext(running_queue->head->context);

	interruptEnable();
}

void threadLock(int lockNum) {
	interruptDisable();

	bool grabLock = false;

	do {
		if (mutex_locks[lockNum] == 0) {
			mutex_locks[lockNum] = 1;
			grabLock = true;
		} else {
			Thread *suspendThread = dequeue(running_queue);
			enqueue(waiting_queue[lockNum][SUSPEND_SLOT], suspendThread);
			swapcontext(suspendThread->context, running_queue->head->context);
		}
	} while (!grabLock);

	interruptEnable();
}

void threadUnlock(int lockNum) {
	interruptDisable();

	mutex_locks[lockNum] = 0;

	Thread *wakeThread = dequeue(waiting_queue[lockNum][SUSPEND_SLOT]);
	if (wakeThread != NULL)
		enqueue(running_queue, wakeThread);

	interruptEnable();
}

void threadWait(int lockNum, int conditionNum) {
	if (mutex_locks[lockNum] == 0) {
		fprintf(stderr, "threadWait(): failed: unlocked mutex");
		exit(-1);
	}

	interruptDisable();

	Thread *waitThread = dequeue(running_queue);

	threadUnlock(lockNum);
	interruptDisable();

	enqueue(waiting_queue[lockNum][conditionNum], waitThread);
	swapcontext(waitThread->context, running_queue->head->context);

	threadLock(lockNum);

	interruptEnable();

}

void threadSignal(int lockNum, int conditionNum) {
	interruptDisable();

	Thread *unblockThread = dequeue(waiting_queue[lockNum][conditionNum]);
	if (unblockThread != NULL)
		enqueue(running_queue, unblockThread);

	interruptEnable();
}

void threadClean(void) {
	interruptDisable();

	free(threadState);

	for (int n = 0; n < NUM_LOCKS; ++n) {
		for (int c = 0; c < NUM_CONDITIONS; ++c) {
			free(waiting_queue[n][c]);
		}
	}

	for (int n = 0; n < threadStateLen; ++n) {
		if (join_queue[n] != NULL)
			free(join_queue[n]);
	}
	free(join_queue);

	interruptEnable();
}
