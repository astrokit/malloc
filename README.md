# Malloc

The program is a POSIX compliant malloc() function implementation in C. The malloc() function in programming languages like C and C++ stands for "memory allocation". It is used to dynamically allocate a specified number of bytes of memory during the execution of a program. This function allows for the creation of variables, arrays, or data structures in the computer's memory at runtime, providing flexibility in managing memory usage.

## About implementation

Allocated memory is saved in a doubly linked list. Each element of the list is a struct with following structure:
* corruption check value for overwlow detection
* data size 
* available flag
* previous memory chunk pointer
* next memory chunk pointer
* data array

The program allocates memory in multiples of memory pages (4096 bytes). If there was allocated more than needed, the current chunk will be resized to the actually requested size. The remaining bytes will be moved to a new chunk for potential later use. The program alos tries to reuse previously allocated free memory chunks before allocating a new chunk.   
The implementation is thread safe. \
The malloc() function returns a pointer to the allocated memory, or NULL if the request fails. 


## Project structure:
* tests
    * advancedtest.cpp
    * errortest.cpp
    * extratest.cpp
    * Makefile
    * newtest.cpp
    * smalltest.cpp
* Dockerfile
* Makefile
* malloc.cpp 
* malloc.h 
* new.cpp


## Usage

The easiest way to set up the project is to build a Docker image based on the provided Dockerfile and Makefile to use it within an interactive bash session. 

    # docker build -t malloc .
    # docker run --rm -it malloc bash
    # cd /app
    # make
