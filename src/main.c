#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "read_line.h"
#include "parse_line.h"
#include "command.h"


int main() {
  char *line;
  char **line_buffer = NULL; // Buffer to store the line for cursor access
  char **args;
  int status;

  do {

    line = read_line();

    if (line == NULL) {
      break; // Exit on EOF (Ctrl+D)
    }
    // save line in a buffer to access with cursor
    line_buffer = realloc(line_buffer, sizeof(char*) * 2);

    if (line_buffer == NULL) {
      fprintf(stderr, "Allocation error\n");
      exit(EXIT_FAILURE);
    }

    line_buffer[0] = line; // Store the current line
    line_buffer[1] = NULL; // Null-terminate the buffer for cursor access
  
    args = parse_line(line);
    status = execute_command(args);
    //
    // printf("Executing command: %s\n", line); // Placeholder for command execution
    // printf("Arguments:\n");
    // for (int i = 0; args[i] != NULL; i++) {
    //   printf("  %s\n", args[i]);
    // }

    //free(line);
    free(args);
  } while (status);

  free(line_buffer); // Free the line buffer if it was allocated
  printf("bye\n");

  return 0;
}

