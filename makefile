CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS =

SRC = src/main.c src/read_line.c src/parse_line.c src/command.c src/history.c
OBJ = $(SRC:.c=.o)
DEPS = src/read_line.h src/constants.h src/parse_line.h src/command.h src/history.h

TARGET = mysh

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

#test: test/test_read_line.c src/read_line.c src/constants.h
#	$(CC) $(CFLAGS) -I./src -o test_read_line test/test_read_line.c src/read_line.c

clean:
	rm -f $(OBJ) $(TARGET) src/*.o

.PHONY: all clean