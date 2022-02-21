CC=gcc
CFLAG=-pedantic -Wall -Werror


TARGETS=proxy
CFLAGS=-std=gnu++17 -pedantic -Wall -Werror -ggdb3 -pthread

all: $(TARGETS)

clean:
	rm -f $(TARGETS) *.o

proxy: proxy.o http_parser.o socket.o threadpool.o log.o cache.o http_base.o http_request.o http_response.o
	g++ -o $@ $^ -lpthread

proxy.o: proxy.cpp 
	g++ $(CFLAGS) -c  $<

socket.o: socket.cpp socket.h 
	g++ $(CFLAGS) -c  $<

http_parser.o: http_parser.cpp http_parser.h http_request.h http_response.h
	g++ $(CFLAGS) -c  $<

threadpool.o: threadpool.cpp threadpool.h 
	g++ $(CFLAGS) -c  $<

cache.o: cache.cpp cache.h 
	g++ $(CFLAGS) -c  $<

log.o: log.cpp log.h
	g++ $(CFLAGS) -c  $<

http_base.o:http_base.cpp http_base.h
	g++ $(CFLAGS) -c  $<

http_request.o:http_request.cpp http_request.h
	g++ $(CFLAGS) -c  $<

http_response.o:http_response.cpp http_response.h
	g++ $(CFLAGS) -c  $<




