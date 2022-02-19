CC=gcc
CFLAG=-pedantic -Wall -Werror


TARGETS=proxy 
CFLAGS=-std=gnu++11 -pedantic -Wall -Werror -ggdb3 -pthread

all: $(TARGETS)

clean:
	rm -f $(TARGETS) *.o

proxy: proxy.o http_parser.o socket.o threadpool.o 
	g++ -o $@ $^ -lpthread

proxy.o: proxy.cpp 
	g++ $(CFLAGS) -c  $<

socket.o: socket.cpp socket.h 
	g++ $(CFLAGS) -c  $<

http_parser.o: http_parser.cpp http_parser.h http_request.h http_response.h
	g++ $(CFLAGS) -c  $<


threadpool.o: threadpool.cpp threadpool.h 
	g++ $(CFLAGS) -c  $<




