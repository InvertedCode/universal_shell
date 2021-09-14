.PHONY: build

gcc := gcc
nasm:= nasm
nasm_args := -f elf64
build: src/main.c src/config.h src/print.S
	$(gcc) -c src/main.c -o build/main.o
	$(gcc) -c src/ssh.c  -o build/ssh.o
	$(nasm) $(nasm_args) src/print.S -o build/print.o
	$(gcc) build/main.o build/print.o build/ssh.o -o ush.exe
