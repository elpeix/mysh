#include "command.h"
#include "alias.h"
#include "cd_command.h"
#include "pipe_commands.h"
#include "constants.h"
#include "history.h"
#include "utils.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int return_free_args(char **args, int subsituted) {
  if (subsituted) {
    for (int i = 0; args[i]; i++) {
      free(args[i]);
    }
  }
  free(args);
  return CONTINUE;
}

int execute_command(char **args) {
  if (args == NULL || args[0] == NULL) {
    return return_free_args(args, 0);
  }

  // Split commands with semicolons
  for (int i = 0; args[i]; i++) {
    if (strcmp(args[i], ";") == 0) {

      // Split the command at the semicolon
      args[i] = NULL;

      // Execute the command before the semicolon
      if (args[0] != NULL) {
        char **command_args = copy_args(args);
        execute_command(command_args);
      }

      // Execute the rest of the commands
      if (args[i + 1] != NULL) {
        char **next_args = copy_args(&args[i + 1]);
        execute_command(next_args);
      }

      return CONTINUE;
    }
  }

  // Substitute the command with its alias if it exists
  int subsituted = subsitute_command(&args);
  if (args[0] == NULL) {
    free_args(args, subsituted);
    return CONTINUE;
  }

  char *command = args[0];

  // exit command
  if (strcmp(command, "exit") == 0) {
    return return_free_args(args, subsituted);
  }

  // cd command
  if (strcmp(command, "cd") == 0) {
    handle_cd(args);
    return return_free_args(args, subsituted);
  }

  // history command
  if (strcmp(command, "history") == 0) {
    print_history();
    return return_free_args(args, subsituted);
  }

  // help command
  if (strcmp(command, "help") == 0) {
    printf("Available commands:\n");
    printf("  help - Show this help message\n");
    printf("  history - Show all history commands\n");
    printf("  mysh --version - Show the version of the shell\n");
    printf("  exit - Exit the shell\n");
    return return_free_args(args, subsituted);
  }

  // version command
  if (strcmp(command, "--version") == 0 ||
      (strcmp(command, "mysh") == 0 && args[1] &&
       strcmp(args[1], "--version") == 0)) {
    printf("mysh version %s\n", MYSH_VERSION);
    return return_free_args(args, subsituted);
  }

  // Pipe command execution
  if (is_pipe_command(args)) {
    int result_pipe_command = execute_pipe_command(args);
    if (result_pipe_command == -1) {
      perror("Pipe command execution failed");
      return return_free_args(args, subsituted);
    }
    if (result_pipe_command == 1) {
      return return_free_args(args, subsituted); // Pipe command executed successfully
    }
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
    return return_free_args(args, subsituted);
  }
  perror("fork");
  return return_free_args(args, subsituted);
}
