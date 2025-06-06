#include "cd_command.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

int handle_cd(char **args) {
    char cwd[PATH_MAX];

    // Get the current working directory
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("cd");
        return 1; // Return error code
    }

    const char *target = NULL;

    if (args[1] == NULL) {
        target = getenv("HOME");
        if (!target) {
            fprintf(stderr, "cd: HOME not set\n");
            return 1; // Return error code
        }
    } else if (strcmp(args[1], "-") == 0) {
        target = getenv("OLDPWD");
        if (!target) {
            fprintf(stderr, "cd: OLDPWD not set\n");
            return 1; // Return error code
        }
        // printf("%s\n", target); // Print the old directory
    } else {
        target = args[1];
    }

    if (chdir(target) != 0) {
        perror("cd");
        return 1; // Return error code
    } else {
        // If chdir is successful, update PWD and OLDPWD
        setenv("OLDPWD", cwd, 1);
        getcwd(cwd, sizeof(cwd));
        setenv("PWD", cwd, 1);
    }
    
    return 0; // Success
}