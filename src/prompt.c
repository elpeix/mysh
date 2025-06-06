#include "prompt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *get_prompt() {

  // Get the current working directory
  char cwd[PATH_MAX] = "?";

  // If getcwd fails, use "?" as a fallback
  if (getcwd(cwd, sizeof(cwd)) == NULL) {
    strcpy(cwd, "?");
  }

  // Change HOME directory to ~
  const char *home = getenv("HOME");
  if (home && strncmp(cwd, home, strlen(home)) == 0) {
    snprintf(cwd, sizeof(cwd), "~%s", cwd + strlen(home));
  }

  static char prompt[PATH_MAX + 10];

  // Format the prompt with colors
  snprintf(prompt, sizeof(prompt), "\033[1;32m%s\033[0m:\033[1;34m%s\033[0m$ ",
           "mysh", cwd);

  // If the prompt is too long, truncate it
  if (strlen(prompt) > PATH_MAX + 10) {
    prompt[PATH_MAX + 10 - 1] = '\0'; // Ensure null-termination
  }

  return prompt;
}
