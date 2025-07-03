
#include "parse_line.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void clear_comments(char *line) {

  if (line == NULL) {
    fprintf(stderr, "Error: NULL line passed to clear_comments\n");
    return;
  }

  // Find the first occurrence of '#' in the line
  char *comment_pos = strchr(line, '#');
  if (comment_pos != NULL) {
    *comment_pos = '\0'; // Replace '#' with null terminator
  }

  // Remove trailing whitespace from the line
  size_t len = strlen(line);
  while (len > 0 && (line[len - 1] == ' ' || line[len - 1] == '\t')) {
    line[len - 1] = '\0'; // Replace trailing whitespace with null terminator
    len--;
  }
}

void insert_spaces_around_semicolons(char *line) {
  char buffer[1024];
  int j = 0;
  for (int i = 0; line[i] != '\0'; i++) {
    if (line[i] == ';') {
      buffer[j++] = ' ';
      buffer[j++] = ';';
      buffer[j++] = ' ';
    } else {
      buffer[j++] = line[i];
    }
  }
  buffer[j] = '\0';
  strcpy(line, buffer);
}

char **parse_line(char *line) {
  if (line == NULL) {
    fprintf(stderr, "Error: NULL line passed to parse_line\n");
    return NULL;
  }

  // Insert spaces around semicolons to ensure proper tokenization
  insert_spaces_around_semicolons(line);

  size_t bufsize = 64;
  char **tokens = malloc(bufsize * sizeof(char *));
  char *token;
  size_t position = 0;

  // Check if memory allocation was successful
  if (!tokens) {
    fprintf(stderr, "Allocation error\n");
    return NULL;
  }

  // Use strtok to split the line into tokens
  token = strtok(line, " \n");

  // Loop through the tokens and add them to the array
  while (token != NULL) {
    tokens[position++] = token;

    // If we have more tokens than the buffer size, reallocate memory
    if (position >= bufsize) {
      bufsize += 64;
      tokens = realloc(tokens, bufsize * sizeof(char *));

      // Check if memory reallocation was successful
      if (!tokens) {
        fprintf(stderr, "Allocation error\n");
        return NULL;
      }
    }

    // Retrieves the next token from the input string, using space and newline
    // as delimiters.
    token = strtok(NULL, " \n");
  }

  tokens[position] = NULL; // Null-terminate the array of tokens

  return tokens;
}
