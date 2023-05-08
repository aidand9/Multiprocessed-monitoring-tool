CC = gcc
CFLAGS =-Werror -Wall -std=c99

main: stats_functions.o main.o
	$(CC) $(CFLAGS) -o $@ $^

stats_functions.o: stats_functions.c stats_functions.h
	$(CC) $(CFLAGS) -c $<

main.o: main.c
	$(CC) $(CFLAGS) -c $<

.PHONY: clean

clean:
	rm *.o
