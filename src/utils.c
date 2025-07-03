#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char **copy_args(char **args) {
  int n = 0;
  while (args[n]) {
    n++;
  }
  char **copy = malloc((n + 1) * sizeof(char *));
  for (int i = 0; i < n; i++) {
    copy[i] = strdup(args[i]);
  }
  copy[n] = NULL;
  return copy;
}

void free_args(char **args, int substituted) {
  if (!args) {
    return;
  }

  if (substituted) {
    for (int i = 0; args[i]; i++) {
      free(args[i]);
    }
  }
  free(args);
}
