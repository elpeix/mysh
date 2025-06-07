
#include "parse_line.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char clear_comments(char *line) {
  if (line == NULL) {
    fprintf(stderr, "Error: NULL line passed to clear_comments\n");
    return '\0';
  }

  char *comment_pos = strchr(line, '#');
  if (comment_pos != NULL) {
    *comment_pos = '\0'; // Replace '#' with null terminator
  }

  return *line; // Return the modified line
}

char **parse_line(char *line) {
  if (line == NULL) {
    fprintf(stderr, "Error: NULL line passed to parse_line\n");
    return NULL;
  }

  size_t bufsize = 64;
  char **tokens = malloc(bufsize * sizeof(char *));
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
      tokens = realloc(tokens, bufsize * sizeof(char *));

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
