#include "pipe_commands.h"
#include "constants.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int is_pipe_command(char **args) {
  if (args == NULL || args[0] == NULL) {
    return 0; // Not a pipe command
  }

  for (int i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], "|") == 0) {
      return 1; // Found a pipe command
    }
  }

  return 0; // Not a pipe command
}

int execute_pipe_chain(char ***cmds, int num_cmds) {
  int prev_fd = -1;

  // Create pipes and fork processes for each command
  for (int i = 0; i < num_cmds; i++) {
    int fd[2];
    // If it's not the last command, create a pipe
    if (i < num_cmds - 1) {
      if (pipe(fd) == -1) {
        perror("pipe");
        exit(1);
      }
    }

    // Fork a new process for the command
    pid_t pid = fork();
    if (pid < 0) {
      perror("fork");
      exit(1);
    }

    if (pid == 0) {

      // Child process
      if (prev_fd != -1) {
        // Redirect standard input to the read end of the previous pipe
        if (dup2(prev_fd, STDIN_FILENO) == -1) {
          perror("dup2");
          exit(1);
        }

        // Close the previous read end in the child process
        close(prev_fd);
      }

      // If this is not the last command in the pipeline
      if (i < num_cmds - 1) {
        // Redirect standard output to the write end of the pipe
        if (dup2(fd[1], STDOUT_FILENO) == -1) {
          perror("dup2");
          exit(1);
        }

        // Close the read end of the pipe in the child process
        close(fd[1]);

        // Close the write end of the pipe in the child process
        close(fd[0]);
      }

      // Execute the command
      execvp(cmds[i][0], cmds[i]);
      perror("execvp");
      exit(1);
    }

    // Parent process
    if (prev_fd != -1) {
      close(prev_fd);
    }

    // If it's not the last command, close the write end of the pipe
    if (i < num_cmds - 1) {
      close(fd[1]);
      prev_fd = fd[0];
    }
  }

  for (int i = 0; i < num_cmds; i++) {
    wait(NULL);
  }

  return 1;
}

int execute_pipe_command(char **args) {
  // Count the number of commands in the pipe
  int num_cmds = 0;
  for (int i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], "|") == 0) {
      num_cmds++;
    }
  }

  num_cmds++; // Add one for the last command after the last pipe
  if (num_cmds < 2) {
    fprintf(stderr, "Invalid pipe command\n");
    return -1; // Invalid pipe command
  }

  // Allocate memory for the commands
  char ***cmds = malloc(sizeof(char **) * num_cmds);
  if (cmds == NULL) {
    perror("malloc");
    return -1; // Memory allocation error
  }

  // Count the number of arguments for each command
  // Assuming all commands have the same number of arguments for simplicity
  int args_per_command = 0;
  for (int i = 0; args[i] != NULL; i++) {
    args_per_command++;
  }

  // Allocate memory for each command
  // We assume a maximum of args_per_command arguments per command
  // plus one for the NULL terminator
  for (int i = 0; i < num_cmds; i++) {
    cmds[i] = malloc(sizeof(char *) * (args_per_command + 1));

    // Check if memory allocation was successful
    if (cmds[i] == NULL) {
      perror("malloc");

      // Free previously allocated commands
      for (int j = 0; j < i; j++) {
        free(cmds[j]);
      }

      free(cmds);

      return -1; // Memory allocation error
    }
  }

  // Fill the commands array
  int cmd_index = 0;
  int arg_index = 0;
  for (int i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], "|") == 0) {
      cmds[cmd_index][arg_index] = NULL; // Null-terminate the current command
      cmd_index++;
      arg_index = 0; // Reset argument index for the next command
    } else {
      cmds[cmd_index][arg_index++] = args[i]; // Add argument to the current command
    }
  }

  cmds[cmd_index][arg_index] = NULL; // Null-terminate the last command
  int result = execute_pipe_chain(cmds, num_cmds);

  // Free the commands array
  for (int i = 0; i < num_cmds; i++) {
    free(cmds[i]);
  }

  free(cmds);

  return result; // Return the result of the pipe command execution
}
