CFLAGS = -g -Wall -Wextra -pedantic
CC = gcc
LD = gcc


all:  mush

mush: mush.o parseline.o
	$(LD) -o mush mush.o parseline.o

mush.o: mush.c mush.h
	$(CC) $(CFLAGS) -c -o mush.o mush.c

parseline.o: parseline.c parseline.h
	$(CC) $(CFLAGS) -c -o parseline.o parseline.c

clean:
	rm -f *.o
