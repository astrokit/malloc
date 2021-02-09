CC=g++
CPPFLAGS=-c -Wall -pthread -g -m32
TITLE=malloc_smalltest

.PHONY : all clean

# make all
all: $(TITLE) test

# make
$(TITLE): malloc.o new.o tests/smalltest.o
	$(CC) -m32 malloc.o new.o tests/smalltest.o -o $(TITLE)

smalltest.o: tests/smalltest.cpp malloc.cpp
	$(CC) $(CPPFLAGS) tests/smalltest.cpp malloc.cpp

malloc.o: malloc.cpp
	$(CC) $(CPPFLAGS) malloc.cpp

new.o: new.cpp
	$(CC) $(CPPFLAGS) new.cpp

test:
	cd ./tests/ && $(MAKE)

# make clean
clean :
	rm -f *.o *.d $(TITLE)
	cd ./tests/ && $(MAKE) clean
