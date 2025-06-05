#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command.h"
#include "constants.h"


int execute_command(char **args) {
  if (args[0] == NULL) {
    return CONTINUE;
  }
  char *command = args[0];

  if (strcmp(command, "exit") == 0) {
    return EXIT;
  }

  if (strcmp(command, "cd") == 0) {
    if (args[1] == NULL) {
      fprintf(stderr, "cd: missing argument\n");
    } else {
      // Simulate change dir
      printf("Changing directory to: %s\n", args[1]);
    }
    return CONTINUE;
  }

  if (strcmp(command, "help") == 0) {
    printf("Available commands:\n");
    printf("  exit - Exit the shell\n");
    printf("  cd <directory> - Change the current directory\n");
    printf("  help - Show this help message\n");
    return CONTINUE;
  }

  // execute program in system
  if (strcmp(command, "system") == 0) {
    if (args[1] == NULL) {
      fprintf(stderr, "system: missing argument\n");
    } else {

      // Use system to execute the command with rest of the arguments
      // Note: This is a simple implementation, it does not handle arguments properly
      // Implode the arguments into a single string
      char command_str[MAX_LINE];
      command_str[0] = '\0';
      for (int i = 1; args[i] != NULL; i++) {
        strcat(command_str, args[i]);
        if (args[i + 1] != NULL) {
          strcat(command_str, " ");
        }
      }
      
      // Execute the command using system
      // Note: This is not secure, it can lead to command injection vulnerabilities
      // In a real shell, you would want to use execvp or similar functions to avoid this issue
      printf("Executing command: %s\n", command_str);

      fflush(stdout); // Ensure the command is printed before executing

      int ret = system(command_str);

      // Check if system call was successful
      if (ret == -1) {
        perror("system");
      }
    }
    return CONTINUE;
  }

  printf("Command not found: %s\n", command);

  return CONTINUE;
}
