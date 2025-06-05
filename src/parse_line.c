
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse_line.h"


char **parse_line(char *line) {
  size_t bufsize = 64;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;
  int position = 0;

  if (!tokens) {
    fprintf(stderr, "Allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, " \n");

  while (token != NULL) {
    tokens[position++] = token;

    if (position >= bufsize) {
      bufsize += 64;
      tokens = realloc(tokens, bufsize * sizeof(char*));

      if (!tokens) {
        fprintf(stderr, "Allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, " \n");
  }

  tokens[position] = NULL; // Null-terminate the array of tokens

  return tokens;
}
