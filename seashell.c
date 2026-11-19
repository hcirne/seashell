#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_ARGS 64

int main(void) {
    char *args[MAX_ARGS];
    char line[1024];

    if (fgets(line, sizeof(line), stdin) == NULL) {
        return 0;
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

    if (args[0] == NULL) {
        return 0;
    }

    if (strcmp(args[0], "exit") == 0) {
        return 0;
    }

    execvp(args[0], args);
    perror("seashell");

    return 1;
}
