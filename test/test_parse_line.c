// test_parse_line.c
#include "../src/parse_line.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_clear_comments() {
  char line[] = "echo hello # this is a comment";
  clear_comments(line);
  assert(strcmp(line, "echo hello ") == 0); // Check if comment is removed
}

void test_clear_comments_no_comment() {
  char line[] = "echo hello";
  clear_comments(line);
  assert(strcmp(line, "echo hello") == 0); // Check if line remains unchanged
}

void test_clear_comments_empty() {
  char line[] = "";
  clear_comments(line);
  assert(strcmp(line, "") == 0); // Check if empty line remains unchanged
}

void test_simple_split() {
  char input[] = "ls -l /tmp";
  char **tokens = parse_line(input);
  assert(tokens != NULL);
  assert(strcmp(tokens[0], "ls") == 0);
  assert(strcmp(tokens[1], "-l") == 0);
  assert(strcmp(tokens[2], "/tmp") == 0);
  assert(tokens[3] == NULL);
  free(tokens);
}

void test_multiple_spaces() {
  char input[] = "echo    hola   peix  ";
  char **tokens = parse_line(input);
  assert(tokens != NULL);
  assert(strcmp(tokens[0], "echo") == 0);
  assert(strcmp(tokens[1], "hola") == 0);
  assert(strcmp(tokens[2], "peix") == 0);
  assert(tokens[3] == NULL);
  free(tokens);
}

void test_empty_line() {
  char input[] = "\n";
  char **tokens = parse_line(input);
  assert(tokens != NULL);
  assert(tokens[0] == NULL);
  free(tokens);
}

void test_only_spaces() {
  char input[] = "     ";
  char **tokens = parse_line(input);
  assert(tokens != NULL);
  assert(tokens[0] == NULL);
  free(tokens);
}

int main() {
  test_simple_split();
  test_multiple_spaces();
  test_empty_line();
  test_only_spaces();
  printf("All parse_line tests passed!\n");
  return 0;
}
