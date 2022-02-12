CC=g++
CFLAG=-pedantic -Wall -Werror
HEADERDIR=../src
LIBDIR=./lib


test_log: test_log.cpp ../src/log.cpp ../src/log.h
	$(CC) $(CFLAGS) -o test_log test_log.cpp ../src/log.cpp -I. -I$(HEADERDIR) -L$(LIBDIR)  -lgtest -lpthread
