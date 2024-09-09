CC=gcc
CFLAGS=-Wall -g

TARGET=coop1 coop2 coop3 preemptive lock1 lock2

all: $(TARGET)

$(TARGET): ../mythreads.c coop1.c coop2.c coop3.c preemptive.c lock1.c lock2.c
	$(CC) $(CFLAGS) ../mythreads.c coop1.c -o coop1
	$(CC) $(CFLAGS) ../mythreads.c coop2.c -o coop2
	$(CC) $(CFLAGS) ../mythreads.c coop3.c -o coop3
	$(CC) $(CFLAGS) ../mythreads.c preemptive.c -o preemptive
	$(CC) $(CFLAGS) ../mythreads.c lock1.c -o lock1
	$(CC) $(CFLAGS) ../mythreads.c lock2.c -o lock2

clean:
	rm $(TARGET)
