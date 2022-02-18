CC=gcc
CFLAG=-pedantic -Wall -Werror


TARGETS=socket threadpool
CFLAGS=-std=gnu++11 -pedantic -Wall -Werror -ggdb3 -pthread

all: $(TARGETS)

clean:
	rm -f $(TARGETS) *.o

socket: socket.o http_parser.o
	g++ -o $@ $^

socket.o: socket.cpp socket.h http_parser.h
	g++ $(CFLAGS) -c  $<

http_parser.o: http_parser.cpp http_parser.h http_request.h http_response.h
	g++ $(CFLAGS) -c  $<

threadpool: threadpool.o 
	g++ -pthread -o $@ $^

threadpool.o: threadpool.cpp threadpool.h 
	g++ $(CFLAGS) -c  $<


