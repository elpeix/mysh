#include "alias.h"
#include "constants.h"
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  char name[MAX_ALIAS_NAME];
  char value[MAX_ALIAS_VALUE];
} Alias;

Alias aliases[MAX_ALIASES];
int alias_count = 0;

void load_aliases() {

  // Load aliases from a file in the user's home directory
  // The file is named ".mysh_alias" and should be in the format:
  // alias name=value
  // Each alias should be on a new line.
  char path[256];
  const char *home = getenv("HOME");

  // If HOME is not set, use the current user's home directory
  if (!home) {
    struct passwd *pw = getpwuid(getuid());
    home = pw ? pw->pw_dir : ".";
  }

  snprintf(path, sizeof(path), "%s/.mysh_alias", home);
  FILE *f = fopen(path, "r");

  // If the file does not exist, we simply return
  if (!f) {
    printf("No alias file found at %s\n", path);
    return;
  }

  // Read the file line by line and parse aliases
  // Each line should start with "alias " followed by the alias name and value
  char line[512];
  while (fgets(line, sizeof(line), f)) {
    if (strncmp(line, "alias ", 6) == 0) {
      char *eq = strchr(line + 6, '=');
      if (eq) {
        *eq = 0;
        char *name = line + 6;
        char *value = eq + 1;

        // Remove line breaks from value
        value[strcspn(value, "\r\n")] = 0;

        // Ensure we do not exceed the maximum number of aliases
        if (alias_count < MAX_ALIASES) {
          // Copy the alias name and value into the aliases array
          strncpy(aliases[alias_count].name, name, MAX_ALIAS_NAME);
          strncpy(aliases[alias_count].value, value, MAX_ALIAS_VALUE);
          alias_count++;
        }
      }
    }
  }
  fclose(f);
}

char *list_aliases() {
  // Return a list of all aliases used for autocomplete commands.
  // If no aliases are defined, return NULL.
  if (alias_count == 0) {
    return NULL; // No aliases defined
  }

  // Allocate memory for the result string, which will hold all aliases
  char *result = malloc(MAX_ALIAS_NAME + 2 * alias_count);
  if (!result) {
    return NULL; // Memory allocation failed
  }

  // Initialize the result string
  result[0] = '\0';
  for (int i = 0; i < alias_count; i++) {
    strcat(result, aliases[i].name);
    if (i < alias_count - 1) {
      strcat(result, " ");
    }
  }

  return result;
}

char *substitute_alias(const char *cmd) {
  // Substitute the command with its alias if it exists
  for (int i = 0; i < alias_count; i++) {
    if (strcmp(cmd, aliases[i].name) == 0) {
      return aliases[i].value;
    }
  }
  return NULL;
}

void free_aliases() {
  // Free the memory allocated for aliases
  for (int i = 0; i < alias_count; i++) {
    memset(aliases[i].name, 0, MAX_ALIAS_NAME);
    memset(aliases[i].value, 0, MAX_ALIAS_VALUE);
  }
  alias_count = 0; // Reset the alias count
}
