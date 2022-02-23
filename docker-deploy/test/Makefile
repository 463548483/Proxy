CC=g++
CFLAGS=-pedantic -Wall -Werror -ggdb3 -Wno-error=sign-compare
HEADERDIR=../src
LIBDIR=./lib


test_log: test_log.cpp ../src/log.cpp ../src/log.h
	$(CC) $(CFLAGS) -o test_log test_log.cpp ../src/log.cpp -I. -I$(HEADERDIR) -L$(LIBDIR)  -lgtest -lpthread
test_http: test_http_parse.cpp ../src/http_response.h ../src/http_request.h ../src/http_parser.h ../src/http_parser.cpp ../src/http_base.cpp ../src/http_response.cpp ../src/http_request.cpp
	$(CC) $(CFLAGS) -o test_http test_http_parse.cpp ../src/http_base.cpp ../src/http_response.cpp ../src/http_request.cpp ../src/http_parser.cpp -I. -I$(HEADERDIR) -L$(LIBDIR)  -lgtest -lpthread
test_threadpool: test_threadpool.cpp ../src/threadpool.cpp ../src/threadpool.h
	$(CC) $(CFLAGS) -o test_threadpool test_threadpool.cpp ../src/threadpool.cpp -I. -I$(HEADERDIR) -L$(LIBDIR)  -lpthread
test_socket: test_socket.cpp ../src/socket.cpp ../src/http_parser.cpp ../src/socket.h ../src/http_parser.h
	$(CC) $(CFLAGS) -o test_socket test_socket.cpp ../src/socket.cpp ../src/http_parser.cpp -I. -I$(HEADERDIR) -L$(LIBDIR)  -lpthread
test_cache: test_cache.cpp ../src/cache.cpp  ../src/cache.h ../src/http_parser.h ../src/http_parser.cpp
	$(CC) $(CFLAGS) -o test_cache test_cache.cpp ../src/cache.cpp ../src/http_parser.cpp -I. -I$(HEADERDIR) -L$(LIBDIR)  


clean:
	rm -f *~ *.o 
