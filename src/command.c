#include "command.h"
#include "constants.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
      if (chdir(args[1]) != 0) {
        perror("cd");
      } else {
        printf("Changed directory to %s\n", args[1]);
      }
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
