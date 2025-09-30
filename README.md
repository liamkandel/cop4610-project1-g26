# Shell

[Description]

## Group Members
- **Liam Kandel**: ltk22@fsu.edu
- **Kaitlyn Franklin**: kmf22f@fsu.edu
- **Ivan Lepesii**: il23d@fsu.edu
## Division of Labor

### Part 1: Prompt
- **Responsibilities**: Implement a dynamic prompt for the shell by expanding $USER, $PWD, and $MACHINE
- **Assigned to**: Liam, Katlyn

### Part 2: Environment Variables
- **Responsibilities**: Implement environment variable expansion so that input such as `$USER,` `$PWD`, etc. is expanded and replaced with its mapped value. This is achieved through `getenv()` system call.
- **Responsibilities**: 
- **Assigned to**: Liam, Kaitlyn, Ivan

### Part 3: Tilde Expansion
- **Responsibilities**: Implement tilde (`~`) expansion so that when a command argument begins with `~` or `~/`, it is replaced with the current user's home directory as specified by the `$HOME` environment variable. 
- **Assigned to**: Liam, Kaitlyn

### Part 4: $PATH Search
- **Responsibilities**: Implement searching for external commands using the directories listed in the `$PATH` environment variable. When a command is entered, the shell parses `$PATH`, splits it into individual directories, and checks each one for an executable file matching the command name using `access()`. If found, the shell uses the full path for execution. 
- **Assigned to**: Liam, Ivan

### Part 5: External Command Execution
- **Responsibilities**: Implement the execution of external commands by forking a child process and using `execv()` to run the target executable. This includes handling arguments, searching for the executable in the directories listed in `$PATH`, and making the parent process wait for the child.
- **Assigned to**: Liam, Ivan

### Part 6: I/O Redirection
- **Responsibilities**: Implement input and output redirection for commands using `<`, `>`, and `>>` operators. This includes parsing redirection tokens, managing file descriptors, applying redirection with `dup2()`, ensuring output files are created with `-rw-------` (0600) permissions, and restoring original stdin/stdout for built-in commands. 
- **Assigned to**: Liam, Kaitlyn

### Part 7: Piping
- **Responsibilities**: Implement support for command pipelines using the `|` operator, allowing the output of one command to be used as the input for the next. This includes parsing commands separated by pipes, setting up the necessary pipes with `pipe()`, forking child processes for each command in the pipeline, redirecting stdin/stdout using `dup2()`, and taking care of unused file descriptors. 
- **Assigned to**: Ivan, Liam

### Part 8: Background Processing
- **Responsibilities**: Implement background processing so that commands ending with `&` are executed without blocking the shell. This includes parsing for the `&` operator, forking child processes with `waitpid()` using `WNOHANG`, tracking up to 10 concurrent background jobs, and providing a `jobs` built-in to list active background processes.
- **Assigned to**: Kaitlyn, Ivan

### Part 9: Internal Command Execution
- **Responsibilities**: Implement execution of internal (built-in) shell commands such as `cd`, `exit`, `jobs`, and `echo`. 
- **Assigned to**: Kaitlyn, Ivan

## File Listing
```
shell/
│
├── src/
│ ├── bg_jobs.c
│ ├── builtins.c
│ ├── env_var_expansion.c
│ ├── external_cmd.c
│ ├── io_redir.c
│ ├── lexer.c
│ ├── path_search.c
│ ├── pipeline.c
│ ├── prompt.c
│ └── tilde_expansion.c
│
├── include/
│ ├── bg_jobs.h
│ ├── builtins.h
│ ├── env_var_expansion.h
│ ├── external_cmd.h
│ ├── io_redir.h
│ ├── lexer.h
│ ├── path_search.h
│ ├── pipeline.h
│ ├── prompt.h
│ └── tilde_expansion.h
│
├── README.md
└── Makefile
```
## How to Compile & Execute

### Requirements
- **Compiler**: e.g., `gcc` for C/C++, `rustc` for Rust.
- **Dependencies**: List any libraries or frameworks necessary (rust only).

### Compilation
For a C/C++ example:
```bash
make
```
This will build the executable in ...
### Execution
```bash
make run
```
This will run the program ...

## Development Log
Each member records their contributions here.

### Liam Kandel

| Date       | Work Completed / Notes |
|------------|------------------------|
| 2025-09-11 | Initialized git repo, began work on Part I  |
| 2025-09-24 | Refactored some parts and organized the code. Made sure it compiled on linprog.  |
| 2025-09-26 | Completed part 5  |
| 2025-09-29 | Separated functions and created header files. Updated README  |

### [Member 2]

| Date       | Work Completed / Notes |
|------------|------------------------|
| YYYY-MM-DD | [Description of task]  |
| YYYY-MM-DD | [Description of task]  |
| YYYY-MM-DD | [Description of task]  |


### Ivan Lepesii

| Date       | Work Completed / Notes |
|------------|------------------------|
| 2025-09-21 | Breaks out expansions into a separate file |
| 2025-09-23 | Env expansion can now expand multiple envs in one token  |
| YYYY-MM-DD | [Description of task]  |


## Meetings
Document in-person meetings, their purpose, and what was discussed.

| Date       | Attendees            | Topics Discussed | Outcomes / Decisions |
|------------|----------------------|------------------|-----------------------|
| YYYY-MM-DD | [Names]              | [Agenda items]   | [Actions/Next steps]  |
| YYYY-MM-DD | [Names]              | [Agenda items]   | [Actions/Next steps]  |



## Bugs
- **Bug 1**: This is bug 1.
- **Bug 2**: This is bug 2.
- **Bug 3**: This is bug 3.


## Considerations
[Description]
# cop4610-project1-g26

