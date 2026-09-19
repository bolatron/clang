CC = gcc
CFLAGS = -Wall -Wextra -g -Iutils
TARGET = main
SRCS = $(wildcard *.c) $(shell find utils/src -name '*.c')
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)
