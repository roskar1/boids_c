default: main.h main.c
	gcc main.c -O3 -Wall -lraylib -o main
clean:
	rm main
