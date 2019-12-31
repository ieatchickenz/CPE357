CC = gcc

C_FLAGS = -Wall -ansi -pedantic -g

all: hencode hdecode

hencode: hencode.o
	$(CC) $(C_FLAGS) -o hencode hencode.o

hdecode: hdecode.o
	$(CC) $(C_FLAGS) -o hdecode hdecode.o

hencode.o: hencode.c
	$(CC) $(C_FLAGS) -c -o hencode.o hencode.c

hdecode.o: hdecode.c
	$(CC) $(C_FLAGS) -c -o hdecode.o hdecode.c

clean:
	rm -f *.o
