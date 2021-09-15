GCC := gcc

.PHONY: all build

all: build

build: src/main.c
	gcc -c src/main.c -o build/main.o
	gcc build/main.o -o ush
	
