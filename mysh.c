// mysh.c — Step 2: I/O redirection (> < >>)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>    // open(), O_WRONLY, O_CREAT etc.

#define MAX_LINE  1024
#define MAX_ARGS  64

int parse(char *line, char **argv, char **infile, char **outfile, int *append) {
    int argc = 0;
    *infile = NULL;
    *outfile = NULL;
    *append = 0;

    char *tok = strtok(line, " \t\n");
    while (tok && argc < MAX_ARGS - 1) {
        if (strcmp(tok, ">") == 0) {
            tok = strtok(NULL, " \t\n");
            *outfile = tok;           // next token is the output file
        } else if (strcmp(tok, ">>") == 0) {
            tok = strtok(NULL, " \t\n");
            *outfile = tok;
            *append = 1;              // append mode
        } else if (strcmp(tok, "<") == 0) {
            tok = strtok(NULL, " \t\n");
            *infile = tok;            // next token is the input file
        } else {
            argv[argc++] = tok;
        }
        tok = strtok(NULL, " \t\n");
    }
    argv[argc] = NULL;
    return argc;
}

int main(void) {
    char  line[MAX_LINE];
    char *argv[MAX_ARGS];
    char *infile, *outfile;
    int   append;

    while (1) {
        printf("mysh> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        int argc = parse(line, argv, &infile, &outfile, &append);
        if (argc == 0) continue;

        pid_t pid = fork();
        if (pid < 0) { perror("fork"); continue; }

        if (pid == 0) {
            // ── Handle input redirection ──
            if (infile) {
                int fd = open(infile, O_RDONLY);
                if (fd < 0) { perror(infile); exit(1); }
                dup2(fd, STDIN_FILENO);   // rewire stdin to the file
                close(fd);
            }

            // ── Handle output redirection ──
            if (outfile) {
                int flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);
                int fd = open(outfile, flags, 0644);
                if (fd < 0) { perror(outfile); exit(1); }
                dup2(fd, STDOUT_FILENO);  // rewire stdout to the file
                close(fd);
            }

            execvp(argv[0], argv);
            perror(argv[0]);
            exit(1);
        }

        waitpid(pid, NULL, 0);
    }
    return 0;
}
