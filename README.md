# Custom-Unix-Shell


A POSIX-style shell built from scratch in C.

## Features (so far)
- Command execution via fork/exec/wait
- I/O redirection (>, <, >>)
- Pipes (cmd1 | cmd2)

## How to build
gcc -Wall -o mysh mysh.c

## How to run
./mysh

##Stages
- commit 1: "Step 1: basic REPL with fork/exec/wait"
- commit 2: "Step 2: I/O redirection with dup2"
- commit 3: "Step 3: pipes"
- commit 4: "Step 4: builtins and signal handling"
- commit 5: "Step 5: background jobs and history"
