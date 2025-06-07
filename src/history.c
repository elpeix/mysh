#include "history.h"
#include "constants.h"
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *history[MAX_HIST];
int history_len = 0;
int history_pos = -1;

void add_to_history(const char *input) {
  // If the input is empty, do not add it to history
  if (input == NULL || input[0] == '\0')
    return;

  // If the input is already in history, do not add it again
  if (history_len > 0 && strcmp(history[history_len - 1], input) == 0) {
    return; // Do not add duplicate entries
  }

  // If history is not full, add the input to history
  if (history_len < MAX_HIST) {
    history[history_len++] = strdup(input);
  } else {
    // If history is full, remove the oldest entry and add the new one
    free(history[0]);
    for (int i = 1; i < MAX_HIST; i++) {
      history[i - 1] = history[i];
    }
    history[MAX_HIST - 1] = strdup(input);
  }
}

void free_history(void) {
  for (int i = 0; i < history_len; i++) {
    free(history[i]);
    history[i] = NULL;
  }
  history_len = 0;
  history_pos = -1;
}

static const char *get_history_path() {
  static char path[512];
  const char *home = getenv("HOME");

  // If HOME is not set, use the current user's home directory
  if (!home) {
    struct passwd *pw = getpwuid(getuid());
    home = pw ? pw->pw_dir : ".";
  }

  // Ensure the path is large enough to hold the full path
  snprintf(path, sizeof(path), "%s/.mysh_history", home);

  return path;
}

void save_history() {
  const char *path = get_history_path();

  FILE *f = fopen(path, "w");
  if (!f) {
    return;
  }

  for (int i = 0; i < history_len; i++) {
    if (history[i]) {
      fprintf(f, "%s\n", history[i]);
    }
  }

  fclose(f);
}

void load_history() {
  const char *path = get_history_path();

  FILE *f = fopen(path, "r");
  if (!f) {
    return;
  }

  char buf[1024];

  while (fgets(buf, sizeof(buf), f)) {
    // Skip empty lines
    if (buf[0] == '\n') {
      continue;
    }

    // Skip comments
    if (buf[0] == '#') {
      continue;
    }

    // Remove the newline character at the end
    buf[strcspn(buf, "\n")] = 0;

    // Add the command to history
    add_to_history(buf);
  }
  fclose(f);
}

void print_history() {
  for (int i = 0; i < history_len; i++) {
    if (history[i]) {
      printf("%s\n", history[i]);
    }
  }
}
