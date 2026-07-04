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
