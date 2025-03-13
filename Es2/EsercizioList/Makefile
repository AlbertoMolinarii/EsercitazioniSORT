CC = gcc
CFLAGS = -g -Wall

all: main

main: main.o list.o
	$(CC) $(CFLAGS) -o main main.o list.o

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f main *.o *~
