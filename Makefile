CC = gcc
AR = ar

CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Iinclude/fluid

TARGET = nuclear_sim
LIB = libflow.a

LDLIBS = -lm

FLOW_SRC = src/fluid/flow.c
FLOW_OBJ = src/fluid/flow.o

MAIN_SRC = src/main.c
MAIN_OBJ = src/main.o

all: $(TARGET)

$(TARGET): $(MAIN_OBJ) $(LIB)
	$(CC) $(MAIN_OBJ) -L. -lflow $(LDLIBS) -o $(TARGET)

$(LIB): $(FLOW_OBJ)
	$(AR) rcs $(LIB) $(FLOW_OBJ)

$(FLOW_OBJ): $(FLOW_SRC) include/fluid/flow.h
	$(CC) $(CFLAGS) -c $(FLOW_SRC) -o $(FLOW_OBJ)

$(MAIN_OBJ): $(MAIN_SRC) include/fluid/flow.h
	$(CC) $(CFLAGS) -c $(MAIN_SRC) -o $(MAIN_OBJ)

clean:
	rm -f $(FLOW_OBJ) $(MAIN_OBJ) $(LIB) $(TARGET)

.PHONY: all clean

