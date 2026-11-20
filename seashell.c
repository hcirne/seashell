#include <fcntl.h>
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

        // Newline loop
        int i = 0;
        while (line[i] != '\0') {
            if (line[i] == '\n') {
                line[i] = '\0';
            }
            i++;
        }

        // Token loop
        char *tok = strtok(line, " ");
        int j = 0;
        while (tok != NULL && j < MAX_ARGS - 1) {
            args[j] = tok;
            tok = strtok(NULL, " ");
            j++;
        }
        args[j] = NULL;

        // redirect loop
        int k = 0;
        char *outfile = NULL;
        char *infile = NULL;
        char **cmd2 = NULL;
        while (k < j) {
            if (strcmp(args[k], "<") == 0 && args[k + 1] != NULL) {
                infile = args[k + 1];
                args[k] = NULL;
            } else if (strcmp(args[k], ">") == 0 && args[k + 1] != NULL) {
                outfile = args[k + 1];
                args[k] = NULL;
            } else if (strcmp(args[k], "|") == 0 && args[k + 1] != NULL) {
                cmd2 = &args[k + 1];
                args[k] = NULL;
            }
            k++;
        }

        // empty command check
        if (args[0] == NULL) {
            continue;
        }

        // exit check
        if (strcmp(args[0], "exit") == 0) {
            break;
        }

        // cd check
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) {
                fprintf(stderr, "seashell: cd: missing argument\n");
            } else if (chdir(args[1]) == -1) {
                perror("seashell: cd");
            }
            continue;
        }

        if (cmd2 != NULL) {
            // pipe block
            int fds[2];

            // creating pipe
            if (pipe(fds) == -1) {
                perror("seashell");
                continue;
            }

            // fork child 1
            pid_t pid1 = fork();
            if (pid1 == 0) {
                dup2(fds[1], 1);
                close(fds[0]);
                close(fds[1]);
                execvp(args[0], args);
                perror("seashell");
                exit(1);
            }

            // fork child 2
            pid_t pid2 = fork();
            if (pid2 == 0) {
                dup2(fds[0], 0);
                close(fds[0]);
                close(fds[1]);
                execvp(cmd2[0], cmd2);
                perror("seashell");
                exit(1);
            }

            // parent: close both pipe ends
            close(fds[0]);
            close(fds[1]);

            // parent: wait for both children
            int status;
            waitpid(pid1, &status, 0);
            waitpid(pid2, &status, 0);

        } else {
            // fork block
            pid_t pid = fork();

            if (pid < 0) {
                perror("seashell");
            } else if (pid == 0) { // child branch
                if (infile != NULL) {
                    int fd = open(infile, O_RDONLY);
                    if (fd == -1) {
                        perror("seashell");
                        exit(1);
                    }
                    dup2(fd, 0);
                    close(fd);
                }
                if (outfile != NULL) {
                    int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd == -1) {
                        perror("seashell");
                        exit(1);
                    }
                    dup2(fd, 1);
                    close(fd);
                }

                execvp(args[0], args);
                perror("seashell");
                exit(1);
            } else { // parent branch
                int status;
                waitpid(pid, &status, 0);
            }
        }
    }
    return 0;
}
