CC=gcc
CFLAG=-pedantic -Wall -Werror


TARGETS=socket
CFLAGS=-std=gnu++11 -pedantic -Wall -Werror -ggdb3 -pthread

all: $(TARGETS)

socket: socket.o 
	g++ -o $@ $^

socket.o: socket.cpp socket.h
	g++ $(CFLAGS) -c  $<

clean:
	rm -f $(TARGETS)