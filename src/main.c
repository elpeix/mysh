#include "alias.h"
#include "command.h"
#include "history.h"
#include "parse_line.h"
#include "read_line.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  char *line;
  char **line_buffer = NULL; // Buffer to store the line for cursor access
  char **args;
  int status;

  // Load history from file
  load_history();

  // Load aliases from file
  load_aliases();

  do {

    line = read_line();

    if (line == NULL) {
      break; // Exit on EOF (Ctrl+D)
    }

    // save line in a buffer to access with cursor
    line_buffer = realloc(line_buffer, sizeof(char *) * 2);

    // Check if realloc was successful
    if (line_buffer == NULL) {
      fprintf(stderr, "Allocation error\n");
      exit(EXIT_FAILURE);
    }

    // Store the current line
    line_buffer[0] = line;

    // Ignore comments
    if (line[0] == '#') {
      continue;
    }

    // Add the line to history
    add_to_history(line);

    // Null-terminate the buffer for cursor access
    line_buffer[1] = NULL;

    // Clear comments
    clear_comments(line);

    args = parse_line(line);
    status = execute_command(args);

    // free(line);
    free(args);
  } while (status);

  // Free the line buffer if it was allocated
  free(line_buffer);

  // Save the history to file
  save_history();

  // Free the history
  free_history();

  // Free aliases
  free_aliases();

  printf("bye\n");

  return 0;
}
