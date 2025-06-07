CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS =

SRC = src/main.c src/read_line.c src/parse_line.c src/command.c src/history.c \
			src/prompt.c src/cd_command.c src/autocomplete.c src/alias.c

OBJ = $(SRC:src/%.c=build/%.o)

DEPS = src/read_line.h src/constants.h src/parse_line.h src/command.h \
			 src/history.h src/prompt.h src/cd_command.h src/autocomplete.h \
       src/alias.h

TARGET = mysh

all: build build/test $(TARGET) test_history test_parse_line

build:
	mkdir -p build

build/test:
	mkdir -p build/test

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

build/%.o: src/%.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

# Tests
build/test/test_history: test/test_history.c src/history.c
	$(CC) $(CFLAGS) -I./src -o build/test/test_history test/test_history.c src/history.c

build/test/test_parse_line: test/test_parse_line.c src/read_line.c
	$(CC) $(CFLAGS) -I./src -o build/test/test_parse_line test/test_parse_line.c src/parse_line.c

test_history: build/test/test_history
test_parse_line: build/test/test_parse_line

clean:
	rm -f $(OBJ) $(TARGET) src/*.o build/test/*

.PHONY: all clean build build/test test_history test_parse_line
