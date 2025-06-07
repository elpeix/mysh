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

    // Ignore comments
    if (line[0] == '#') {
      free(line);
      continue;
    }

    add_to_history(line);
    clear_comments(line);

    args = parse_line(line);
    if (args == NULL || args[0] == NULL) {
      free(line);
      continue; // Skip empty lines or parsing errors
    }

    status = execute_command(args);

    free(args);
    free(line);
  } while (status);

  save_history();
  free_history();
  free_aliases();

  printf("bye\n");

  return 0;
}
