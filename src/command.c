#include "command.h"
#include "alias.h"
#include "cd_command.h"
#include "constants.h"
#include "history.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void subsitute_command(char ***args_ptr) {
  char **args = *args_ptr;

  // Substitute the command with its alias if it exists
  char *alias = substitute_alias(args[0]);
  if (!alias) {
    return;
  }

  // Split the alias into tokens and create a new args array
  char *alias_copy = strdup(alias);
  char *token = strtok(alias_copy, " ");
  char **new_args = malloc(sizeof(char *) * 256);
  int n = 0;

  // Add the first token (the command itself)
  while (token) {
    new_args[n++] = strdup(token);
    token = strtok(NULL, " ");
  }

  // Add the rest of the original arguments, if any
  for (int i = 1; args[i]; i++) {
    new_args[n++] = strdup(args[i]);
  }
  new_args[n] = NULL;

  // Copy the new arguments back to the original args array
  *args_ptr = new_args;

  free(alias_copy);
}

int execute_command(char **args) {
  if (args[0] == NULL) {
    return CONTINUE;
  }
  // Substitute the command with its alias if it exists
  subsitute_command(&args);
  if (args[0] == NULL) {
    return CONTINUE; // If the command was substituted to NULL, do nothing
  }

  char *command = args[0];

  // exit command
  if (strcmp(command, "exit") == 0) {
    return EXIT;
  }

  // cd command
  if (strcmp(command, "cd") == 0) {
    handle_cd(args);
    return CONTINUE;
  }

  // history command
  if (strcmp(command, "history") == 0) {
    print_history();
    return CONTINUE;
  }

  // help command
  if (strcmp(command, "help") == 0) {
    printf("Available commands:\n");
    printf("  help - Show this help message\n");
    printf("  history - Show all history commands\n");
    printf("  mysh --version - Show the version of the shell\n");
    printf("  exit - Exit the shell\n");
    return CONTINUE;
  }

  // version command
  if (strcmp(command, "--version") == 0 ||
      (strcmp(command, "mysh") == 0 && args[1] &&
       strcmp(args[1], "--version") == 0)) {
    printf("mysh version %s\n", MYSH_VERSION);
    return CONTINUE;
  }

  // Pipe command execution
  int pipe_pos = -1;
  for (int i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], "|") == 0) {
      pipe_pos = i;
      break;
    }
  }

  if (pipe_pos != -1) {
    // Split the arguments into two parts at the pipe position
    args[pipe_pos] = NULL;
    char **left = args;
    char **right = &args[pipe_pos + 1];

    // Create a pipe
    int fd[2];
    pipe(fd);

    // First child process. Write to the pipe
    if (fork() == 0) {
      dup2(fd[1], STDOUT_FILENO);
      close(fd[0]);
      close(fd[1]);
      execvp(left[0], left);
      perror("execvp");
      exit(1);
    }

    // Second child process. This will read from the pipe
    if (fork() == 0) {
      dup2(fd[0], STDIN_FILENO);
      close(fd[0]);
      close(fd[1]);
      execvp(right[0], right);
      perror("execvp");
      exit(1);
    }

    // Parent process. Close the pipe and wait for children
    close(fd[0]);
    close(fd[1]);
    wait(NULL);
    wait(NULL);
    return CONTINUE;
  }

  // Background command execution
  int background = 0;
  int last = 0;
  while (args[last] != NULL)
    last++;
  if (last > 0 && strcmp(args[last - 1], "&") == 0) {
    background = 1;
    args[last - 1] = NULL; // Remove the '&' from the arguments
  }

  // Execute the command using execvp
  char *infile = NULL, *outfile = NULL;
  int i = 0, j = 0;
  while (args[i] != NULL) {
    if (strcmp(args[i], "<") == 0 && args[i + 1]) {
      infile = args[i + 1];
      i += 2;
    } else if (strcmp(args[i], ">") == 0 && args[i + 1]) {
      outfile = args[i + 1];
      i += 2;
    } else {
      args[j++] = args[i++];
    }
  }
  args[j] = NULL;

  pid_t pid = fork();
  if (pid == 0) {
    if (infile) {
      int fd = open(infile, O_RDONLY);
      if (fd < 0) {
        perror("open infile");
        exit(1);
      }
      dup2(fd, STDIN_FILENO);
      close(fd);
    }
    if (outfile) {
      int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (fd < 0) {
        perror("open outfile");
        exit(1);
      }
      dup2(fd, STDOUT_FILENO);
      close(fd);
    }
    execvp(command, args);
    perror("MyShell error");
    exit(1);
  } else if (pid > 0) {
    if (!background) {
      int status;
      waitpid(pid, &status, 0);
    } else {
      printf("Command running in background with PID: %d\n", pid);
    }
    return CONTINUE;
  }
  perror("fork");
  return CONTINUE;
}
