CC=g++
CFLAG=-pedantic -Wall -Werror
HEADERDIR=../src
LIBDIR=./lib


test_log: test_log.cpp ../src/log.cpp ../src/log.h
	$(CC) $(CFLAGS) -o test_log test_log.cpp ../src/log.cpp -I. -I$(HEADERDIR) -L$(LIBDIR)  -lgtest -lpthread
test_http: test_http_parse.cpp ../src/http_response.h ../src/http_request.h ../src/http_parser.h ../src/http_parser.cpp
	$(CC) $(CFLAGS) -o test_http test_http_parse.cpp ../src/http_parser.cpp -I. -I$(HEADERDIR) -L$(LIBDIR)  -lgtest -lpthread

clean:
	rm -f *~ *.o 
