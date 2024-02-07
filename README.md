# Malloc

Thread-safe implementation of the memory management functions malloc() and free().

## Usage

The easiest way to set up the project is to build a Docker image based on the provided `Dockerfile` and `Makefile` to use it in an interactive bash session:

```bash
$ docker build -t malloc .
$ docker run --rm -it malloc bash

$ cd /app
$ make

$ ./tests/advancedtest
```
