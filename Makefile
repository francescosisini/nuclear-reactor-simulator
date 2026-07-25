CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11

TARGET = nuclear_sim
SOURCES = src/main.c

all:
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)
