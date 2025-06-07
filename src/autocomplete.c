#include "autocomplete.h"
#include "alias.h"
#include "constants.h"
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// List of custom commands to autocomplete in the first word
const char *custom_commands[] = {"cd", "help", "history", "exit", NULL};

int autocomplete_in_first_word(char *line, int pos) {
  // Check if the position is at the start of the line
  if (pos == 0) {
    return 0;
  }

  int start = pos - 1;
  while (start >= 0 && line[start] != ' ') {
    start--;
  }
  start++;

  // If the position is at the start of the line, it means we are in the first
  // word.
  if (start == 0) {
    // If the first character is a dot or a slash, we are likely in a path
    if (line[0] == '.' || line[0] == '/') {
      return 0;
    }
    return 1;
  }

  // We are not in the first word.
  return 0;
}

int autocomplete_exec(char *line, int pos) {
  // Find the beginning of the word
  int start = pos - 1;
  while (start >= 0 && line[start] != ' ') {
    start--;
  }
  start++;
  char prefix[256] = {0};
  strncpy(prefix, line + start, pos - start);

  // Get the PATH environment variable
  char *path_env = getenv("PATH");
  if (!path_env) {
    return -1;
  }

  char *paths = strdup(path_env);
  char *saveptr = NULL;
  char *dir = strtok_r(paths, ":", &saveptr);

  char matches_arr[128][256];
  int matches = 0;

  // Add custom commands to the matches array
  for (int i = 0; custom_commands[i]; i++) {
    if (strncmp(custom_commands[i], prefix, strlen(prefix)) == 0) {
      strcpy(matches_arr[matches++], custom_commands[i]);
    }
  }

  // Add aliases to the matches array
  char *aliases = list_aliases();
  if (aliases) {
    char *alias = strtok(aliases, " ");
    while (alias) {
      if (strncmp(alias, prefix, strlen(prefix)) == 0) {
        strcpy(matches_arr[matches++], alias);
      }
      alias = strtok(NULL, " ");
    }
    free(aliases);
  }

  // Search for matching executables in the PATH directories
  while (dir) {
    DIR *d = opendir(dir);
    if (d) {
      struct dirent *entry;
      while ((entry = readdir(d))) {
        if (strncmp(entry->d_name, prefix, strlen(prefix)) == 0) {
          // Prevent duplicates
          int duplicate = 0;
          for (int i = 0; i < matches; i++) {
            if (strcmp(matches_arr[i], entry->d_name) == 0) {
              duplicate = 1;
              break;
            }
          }
          if (!duplicate && matches < 128) {
            strcpy(matches_arr[matches++], entry->d_name);
          }
        }
      }
      closedir(d);
    }
    dir = strtok_r(NULL, ":", &saveptr);
  }
  free(paths);

  if (matches == 1) {

    int extra = strlen(matches_arr[0]) - strlen(prefix) + 1; // +1 for space
    if (pos + extra < MAX_LINE - 1) {
      memmove(line + pos + extra, line + pos, strlen(line + pos) + 1);
      memcpy(line + pos, matches_arr[0] + strlen(prefix), extra);
      pos += extra;
      line[pos++] = ' '; // Add a space after the command
      return pos;
    }

  } else if (matches > 1) {
    // Print all matches
    printf("\nPossible completions:");
    for (int i = 0; i < matches; i++) {
      printf("\n%s", matches_arr[i]);
    }

    // Check for the longest common prefix
    int common_length = strlen(prefix);
    int done = 0;
    while (!done) {
      char current_char = matches_arr[0][common_length];
      if (current_char == '\0') {
        break;
      }
      for (int i = 1; i < matches; i++) {
        if (matches_arr[i][common_length] != current_char) {
          current_char = 0;
          break;
        }
      }
      if (current_char) {
        common_length++;
      } else {
        done = 1;
      }
    }
    if (common_length > strlen(prefix)) {
      int extra = common_length - strlen(prefix);
      if (pos + extra < MAX_LINE - 1) {
        memmove(line + pos + extra, line + pos, strlen(line + pos) + 1);
        memcpy(line + pos, matches_arr[0] + strlen(prefix), extra);
        pos += extra;
      }
    }
    printf("\n");
    return pos;
  }
  return -1;
}

int autocomplete_path(char *line, int pos) {
  // Find the beginning of the word
  int start = pos - 1;
  while (start >= 0 && line[start] != ' ' && line[start] != '/') {
    start--;
  }
  start++;
  char prefix[256] = {0};
  strncpy(prefix, line + start, pos - start);

  // Search for matching files in the current directory
  char dirpath[PATH_MAX] = ".";
  int slash = start - 1;
  while (slash >= 0 && line[slash] != ' ') {
    slash--;
  }
  if (slash + 1 < start) {
    strncpy(dirpath, line + slash + 1, start - slash - 1);
    dirpath[start - slash - 1] = '\0';
  }

  DIR *dir = opendir(dirpath);
  if (!dir) {
    return -1; // No directory found
  }

  struct dirent *entry;
  char match[256] = {0};
  char matches_arr[64][256];
  int matches = 0;

  while ((entry = readdir(dir))) {
    if (strncmp(entry->d_name, prefix, strlen(prefix)) == 0) {
      if (matches < 64) {
        strcpy(matches_arr[matches], entry->d_name);
      }
      if (matches == 0) {
        strcpy(match, entry->d_name);
      } else {
        // If there are multiple matches, we will not complete the word
        match[0] = '\0';
      }
      matches++;
    }
  }
  closedir(dir);

  if (matches == 1 && match[0]) {
    // Check if the match is a directory
    char fullpath[PATH_MAX];

    // Check if the full path is too long (+1: '/' and +1: '\0')
    if (strlen(dirpath) + 1 + strlen(match) + 1 > sizeof(fullpath)) {
      fprintf(stderr, "Path too long for completion\n");
      return -1;
    }
    snprintf(fullpath, sizeof(fullpath), "%s/%s", dirpath, match);
    struct stat st;
    char last_char = ' ';
    if (stat(fullpath, &st) == 0 && S_ISDIR(st.st_mode)) {
      last_char = '/'; // Add a slash if it's a directory
    }

    // Complete the word
    int extra = strlen(match) - strlen(prefix) + 1; // +1 for slash or space
    if (pos + extra < MAX_LINE - 1) {
      memmove(line + pos + extra, line + pos, strlen(line + pos) + 1);
      memcpy(line + pos, match + strlen(prefix),
             strlen(match) - strlen(prefix));
      pos += strlen(match) - strlen(prefix);

      line[pos++] = last_char;

      return pos;
    }
  } else if (matches > 1) {
    // Print all matches
    printf("\nPossible completions:");
    for (int i = 0; i < matches; i++) {
      printf("\n%s", matches_arr[i]);
    }

    // Find the longest common prefix
    int common_length = strlen(prefix);
    int done = 0;
    while (!done) {
      // Store the current matches in the array
      char current_char = matches_arr[0][common_length];

      // Break if we reach the end of the string
      if (current_char == '\0') {
        break;
      }

      // Check if all matches have the same character at the current position
      for (int i = 0; i < matches; i++) {
        if (matches_arr[i][common_length] != current_char) {
          current_char = 0;
          break;
        }
      }

      // If all matches have the same character, increment the common length
      if (current_char) {
        common_length++;
      } else {
        done = 1; // Stop when characters differ
      }
    }

    // If the common prefix is longer than the original prefix, complete it
    if (common_length > strlen(prefix)) {
      int extra = common_length - strlen(prefix);
      if (pos + extra < MAX_LINE - 1) {
        memmove(line + pos + extra, line + pos, strlen(line + pos) + 1);
        memcpy(line + pos, matches_arr[0] + strlen(prefix), extra);
        pos += extra;
      }
    }
    printf("\n");
    return pos;
  }
  return -1; // No unique match found
}
