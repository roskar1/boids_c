main: main.c
	gcc main.c -Wall -g -I -L -lraylib -o main
clean:
	rm main

