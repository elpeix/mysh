#include "history.h"
#include <stdlib.h>
#include <string.h>

char *history[MAX_HIST];
int history_len = 0;
int history_pos = -1;

void add_to_history(const char *input) {
  if (history_len < MAX_HIST) {
    history[history_len++] = strdup(input);
  } else {
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
