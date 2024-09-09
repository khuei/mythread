CC=gcc
CFLAGS=-Wall -g

TARGET=libmythreads.a

all: $(TARGET)

libmythreads.a: mythreads.c
	$(CC) $(CFLAGS) -c mythreads.c
	ar -cvrs $(TARGET) mythreads.o

clean:
	rm mythreads.o
	rm $(TARGET)
