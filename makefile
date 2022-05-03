CC:=gcc
CFLAGS:=-s -Wall -O3

all: hangman

dict.o: utils/dict.c
	$(CC) $(CFLAGS) -c $^

hangman: dict.o main.c
	$(CC) $(CFLAGS) -o $@ $^ -lncurses

clean:
	rm hangman *.o
