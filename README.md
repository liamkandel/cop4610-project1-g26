# Shell

[Description]

## Group Members
- **Liam Kandel**: ltk22@fsu.edu
- **Kaitlyn Franklin**: kmf22f@fsu.edu
- **Ivan Lepesii**: il23d@fsu.edu
## Division of Labor

### Part 1: Prompt
- **Responsibilities**: Implement a dynamic prompt for the shell using $USER, $PWD, and $MACHINE
- **Assigned to**: Liam, Katlyn

### Part 2: Environment Variables
- **Responsibilities**: Expand environment variables using system calls like getenv()
- **Assigned to**: Liam, Kaitlyn, Ivan

### Part 3: Tilde Expansion
- **Responsibilities**: Expand out ~ and ~/ to absolute working directory
- **Assigned to**: Liam, Kaitlyn

### Part 4: $PATH Search
- **Responsibilities**: Search PATH directories for an executable given as the first token
- **Assigned to**: Liam, Ivan

### Part 5: External Command Execution
- **Responsibilities**: Execute command using fork() and execv()
- **Assigned to**: Liam, Ivan

### Part 6: I/O Redirection
- **Responsibilities**: [Description]
- **Assigned to**: Liam, Kaitlyn

### Part 7: Piping
- **Responsibilities**: [Description]
- **Assigned to**: Ivan, Liam

### Part 8: Background Processing
- **Responsibilities**: [Description]
- **Assigned to**: Kaitlyn, Ivan

### Part 9: Internal Command Execution
- **Responsibilities**: [Description]
- **Assigned to**: Kaitlyn, Ivan

### Part 10: External Timeout Executable
- **Responsibilities**: [Description]
- **Assigned to**: Alex Brown, Jane Smith

## File Listing
```
shell/
│
├── src/
│ ├── main.c
│ ├── expand.c
│ └── lexer.c
│
├── include/
│ ├── expand.h
│ └── lexer.h
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

## Extra Credit
- **Extra Credit 1**: [Extra Credit Option]
- **Extra Credit 2**: [Extra Credit Option]
- **Extra Credit 3**: [Extra Credit Option]

## Considerations
[Description]
# cop4610-project1-g26
