# mysh — A Custom Unix Shell in C

A POSIX-style shell built from scratch in C, implementing core Unix shell
features using low-level system calls. Built as a learning project while
studying Operating Systems: Three Easy Pieces (OSTEP).

---

## Features

- Command execution — runs any program via `fork/exec/wait`
- I/O redirection — `>`, `<`, `>>` using `open()` and `dup2()`
- Pipes — `cmd1 | cmd2` using `pipe()` and `dup2()`
- Builtins — `cd` and `exit` run directly in the shell process
- Signal handling — `Ctrl+C` kills the running command but not the shell
- Background jobs — `cmd &` runs a command without blocking the shell
- Command history — press `↑` `↓` to navigate previous commands

---

## Project structure

myshell/

├── mysh.c       # shell source code

└── README.md    
---

## Build

```bash
gcc -Wall -o mysh mysh.c
```

## Run

```bash
./mysh
```

---

## Usage

```bash
# basic commands
mysh> ls -la
mysh> echo hello world
mysh> pwd

# I/O redirection
mysh> echo hello > out.txt       # write to file
mysh> cat < out.txt              # read from file
mysh> echo second line >> out.txt  # append to file

# pipes
mysh> ls | grep mysh
mysh> echo hello world | grep hello
mysh> cat file.txt | sort | uniq

# builtins
mysh> cd /tmp        # change directory
mysh> cd             # go to home directory
mysh> exit           # exit the shell

# background jobs
mysh> sleep 10 &     # runs in background, prompt returns immediately
mysh> ls             # shell is free to run other commands

# signal handling
mysh> sleep 10       # press Ctrl+C — kills sleep, shell survives
```

---

## What I learned

### Process management
`fork()` creates a child process that is an exact copy of the parent —
same code, same memory, same file descriptors. `exec()` then replaces the
child's memory image with a new program. The parent survives because only
the child is replaced. Without this fork+exec separation, running any
command would destroy the shell itself.

### File descriptors
Every process starts with three open file descriptors: fd 0 (stdin),
fd 1 (stdout), fd 2 (stderr), all pointing at the terminal by default.
`dup2(fd, 1)` rewires stdout to point to a file instead — so programs
like `echo` write to a file without knowing it. The program just writes
to fd 1 as always.

### Pipes
A pipe is a kernel buffer with two ends — a write end and a read end.
`ls | grep` works by wiring `ls`'s stdout to the pipe write end and
`grep`'s stdin to the pipe read end via `dup2()`. Both run simultaneously
as separate child processes. Unused pipe ends must be closed in every
process, otherwise the reader blocks forever waiting for EOF that never
comes.

### Builtins
Commands like `cd` cannot be forked — if a child process calls
`chdir()`, only the child's directory changes. The parent shell is
unaffected. So `cd` must run directly inside the shell process itself,
with no forking at all. Same for `exit` — if a child exited, the shell
would keep running.

### Signal handling
`Ctrl+C` sends `SIGINT` to the entire process group — both the shell and
any running child. The shell ignores `SIGINT` via `SIG_IGN` so it
survives. Children restore the default handler via `SIG_DFL` before
`exec` so they can be killed normally. Without this restore, children
would inherit the parent's `SIG_IGN` and become unkillable.

### Background jobs
Normally `waitpid()` blocks the parent until the child finishes. For
background jobs (`cmd &`), we skip `waitpid()` so the shell returns the
prompt immediately. To avoid zombie processes, we handle `SIGCHLD` —
the signal the kernel sends when any child exits — and call `waitpid()`
inside the handler to reap finished background children automatically.

---

## System calls used

| Syscall | Purpose |
|---|---|
| `fork()` | create a child process |
| `execvp()` | replace child's memory with a new program |
| `waitpid()` | parent waits for child to finish |
| `pipe()` | create a unidirectional communication channel |
| `dup2()` | rewire file descriptors |
| `open()` | open a file and get a file descriptor |
| `chdir()` | change working directory (cd builtin) |
| `signal()` | register signal handlers |
| `getenv()` | read environment variables (HOME for cd) |



---

## Built with

- C (C99)
- GCC
- WSL — Ubuntu on Windows

---

## References

- [Operating Systems: Three Easy Pieces](https://ostep.org) — Chapters 4, 5, 39
- `man fork`, `man execvp`, `man pipe`, `man dup2`, `man signal`
- [Beej's Guide to Unix IPC](https://beej.us/guide/bgipc/)

---


