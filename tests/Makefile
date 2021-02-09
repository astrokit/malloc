CC=g++
CPPFLAGS=-Wall -g -pthread -m32

.PHONY : all clean

SRCS=$(wildcard *.cpp)
EXECUTABLES=$(SRCS:.cpp= )
OBJ=$(SRCS:.cpp=.o)

all: ${EXECUTABLES}

${EXECUTABLES}: ${OBJ}
	$(CC) $(CPPFLAGS) ../malloc.o $@.o -o $@

${OBJ}: ${SRCS}
	$(CC) -c $(CPPFLAGS) $(@:.o=.cpp) -o $@

clean: 
	rm -f *.o ${EXECUTABLES}

