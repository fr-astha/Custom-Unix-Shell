#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>    // signal(), SIG_IGN, SIG_DFL, SIGINT

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
            *outfile = tok;
        } else if (strcmp(tok, ">>") == 0) {
            tok = strtok(NULL, " \t\n");
            *outfile = tok;
            *append = 1;
        } else if (strcmp(tok, "<") == 0) {
            tok = strtok(NULL, " \t\n");
            *infile = tok;
        } else {
            argv[argc++] = tok;
        }
        tok = strtok(NULL, " \t\n");
    }
    argv[argc] = NULL;
    return argc;
}

void exec_cmd(char **argv, char *infile, char *outfile, int append) {
    // restore SIGINT in child so it can be killed by Ctrl+C
    signal(SIGINT, SIG_DFL);

    if (infile) {
        int fd = open(infile, O_RDONLY);
        if (fd < 0) { perror(infile); exit(1); }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    if (outfile) {
        int flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);
        int fd = open(outfile, flags, 0644);
        if (fd < 0) { perror(outfile); exit(1); }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    execvp(argv[0], argv);
    perror(argv[0]);
    exit(1);
}

int main(void) {
    char  line[MAX_LINE];

    // parent ignores Ctrl+C 
    signal(SIGINT, SIG_IGN);

    while (1) {
        printf("mysh> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        
        char *pipe_pos = strchr(line, '|');

        if (pipe_pos) {
            *pipe_pos = '\0';
            char *left  = line;
            char *right = pipe_pos + 1;

            char *argv1[MAX_ARGS], *argv2[MAX_ARGS];
            char *in1, *out1, *in2, *out2;
            int   app1, app2;
            parse(left,  argv1, &in1, &out1, &app1);
            parse(right, argv2, &in2, &out2, &app2);

            int pipefd[2];
            if (pipe(pipefd) < 0) { perror("pipe"); continue; }

            pid_t pid1 = fork();
            if (pid1 == 0) {
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
                exec_cmd(argv1, in1, out1, app1);
            }

            pid_t pid2 = fork();
            if (pid2 == 0) {
                close(pipefd[1]);
                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[0]);
                exec_cmd(argv2, in2, out2, app2);
            }

            close(pipefd[0]);
            close(pipefd[1]);
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);

        } else {
            char *argv[MAX_ARGS];
            char *infile, *outfile;
            int   append;
            int argc = parse(line, argv, &infile, &outfile, &append);
            if (argc == 0) continue;

            // ── builtins ──────────────────────────────
            if (strcmp(argv[0], "cd") == 0) {
                char *path = argv[1] ? argv[1] : getenv("HOME");
                if (chdir(path) < 0)
                    perror("cd");
                continue;
            }

            if (strcmp(argv[0], "exit") == 0) {
                exit(0);
            }
          

            pid_t pid = fork();
            if (pid < 0) { perror("fork"); continue; }
            if (pid == 0) exec_cmd(argv, infile, outfile, append);
            waitpid(pid, NULL, 0);
        }
    }
    return 0;
}
