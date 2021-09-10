.PHONY: build

gcc := gcc

build: src/main.c src/config.h src/print.S
	$(gcc) -c src/main.c -o build/main.o
	$(gcc) -c src/print.S -o build/print.o
	$(gcc) build/main.o build/print.o -o ush.exe