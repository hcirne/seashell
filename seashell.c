#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_ARGS 64

int main(void) {
    char *args[MAX_ARGS];
    char line[1024];

    while (1) {

        printf("seashell> ");
        fflush(stdout);

        // Ctrl + D input
        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            break;
        }

        int i = 0;
        while (line[i] != '\0') {
            if (line[i] == '\n') {
                line[i] = '\0';
            }
            i++;
        }
        char *tok = strtok(line, " ");
        int j = 0;
        while (tok != NULL && j < MAX_ARGS - 1) {
            args[j] = tok;
            tok = strtok(NULL, " ");
            j++;
        }
        args[j] = NULL;

        // Input is blank
        if (args[0] == NULL) {
            continue;
        }

        // Input is "exit"
        if (strcmp(args[0], "exit") == 0) {
            break;
        }

        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) {
                fprintf(stderr, "seashell: cd: missing argument\n");
            } else if (chdir(args[1]) == -1) {
                perror("seashell: cd:");
            }
            continue;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("seashell");
        } else if (pid == 0) {
            execvp(args[0], args);
            perror("seashell");
            exit(1);
        } else {
            int status;
            waitpid(pid, &status, 0);
        }
    }

    return 0;
}
