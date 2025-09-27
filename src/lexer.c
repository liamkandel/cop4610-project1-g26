#include "lexer.h"
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

int parse_redirection(tokenlist *tokens, redir_t *r); /* removes redir tokens from tokens */
int apply_redirection(const redir_t *r, int *saved_stdin, int *saved_stdout); /* dup2s; returns 0 on success */
void restore_stdio(int saved_stdin, int saved_stdout);
int builtin_echo(tokenlist *tokens);

tokenlist* environment_variable_expansion(tokenlist*tokens); // --- PART 2: ENVIRONMENT VARIABLE EXPANSION ---
tokenlist* tilde_expansion(tokenlist* tokens); // --- PART 3: TILDE EXPANSION ---
// implement path search function
char* path_search(char* tokens); // --- PART 4: PATH SEARCH ---
void execute_pipeline(tokenlist **commands, int num_commands); // --- PART 7: PIPING ---


/* helper strdup replacement to avoid implicit decl on some platforms */
static char *xstrdup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (!p) return NULL;
    memcpy(p, s, n);
    return p;
}

/* Parse pipes and split tokenlist into separate commands */
int parse_pipes(tokenlist *tokens, tokenlist ***commands) {
    int num_commands = 1;
    
    /* Count pipes */
    for (int i = 0; i < tokens->size; i++) {
        if (strcmp(tokens->items[i], "|") == 0) {
            num_commands++;
        }
    }
    
    if (num_commands == 1) return 0; /* No pipes */
    if (num_commands > 3) return -1; /* Too many pipes */
    
    /* Allocate and split commands */
    *commands = malloc(num_commands * sizeof(tokenlist*));
    for (int i = 0; i < num_commands; i++) {
        (*commands)[i] = new_tokenlist();
    }
    
    int current_cmd = 0;
    for (int i = 0; i < tokens->size; i++) {
        if (strcmp(tokens->items[i], "|") == 0) {
            current_cmd++;
        } else {
            add_token((*commands)[current_cmd], tokens->items[i]);
        }
    }
    
    return num_commands;
}

int main()
{
	while (1) {
		
        // ---- PART 1: PROMPT -----
        const char* user = getenv("USER");
        const char* cwd = getenv("PWD");
        const char* machine = getenv("MACHINE");

        printf("%s@%s:%s$ ", user, machine, cwd);


		/* input contains the whole command
		 * tokens contains substrings from input split by spaces
		 */

		char *input = get_input();
		printf("whole input: %s\n", input);

        /* Skip empty input */
        if (!input || strlen(input) == 0) {
            free(input);
            continue;
        }

        tokenlist *tokens = get_tokens(input);
        if (tokens->size == 0) {
            free(input);
            free_tokens(tokens);
            continue;
        }
        
        tokens = environment_variable_expansion(tokens);
        tokens = tilde_expansion(tokens);

        
        for (int i = 0; i < tokens->size; i++) {
            printf("token %d: (%s)\n", i, tokens->items[i]);
		}

        /* Check for pipes first */
        tokenlist **pipe_commands = NULL;
        int num_commands = parse_pipes(tokens, &pipe_commands);
        
        if (num_commands > 1) {
            /* Handle piped commands - find paths for all commands */
            for (int i = 0; i < num_commands; i++) {
                char *cmdpath = path_search(pipe_commands[i]->items[0]);
                if (cmdpath) {
                    free(pipe_commands[i]->items[0]);
                    pipe_commands[i]->items[0] = cmdpath;
                }
            }
            execute_pipeline(pipe_commands, num_commands);
            
            /* Cleanup */
            for (int i = 0; i < num_commands; i++) {
                free_tokens(pipe_commands[i]);
            }
            free(pipe_commands);
        } else {
            /* Handle single command with redirection */
            redir_t redir;
            if (parse_redirection(tokens, &redir) != 0) {
                /* parse error: cleanup and continue */
                free(input);
                free_tokens(tokens);
                free(redir.in_file);
                free(redir.out_file);
                continue;
            }
            
            /* builtin handling */
            if (strcmp(tokens->items[0], "echo") == 0) {
                int saved_in = -1, saved_out = -1;
                if (apply_redirection(&redir, &saved_in, &saved_out) != 0) {
                    /* failed to apply redirection for builtin */
                    restore_stdio(saved_in, saved_out);
                    free(input);
                    free_tokens(tokens);
                    free(redir.in_file);
                    free(redir.out_file);
                    continue;
                }
                builtin_echo(tokens);
                restore_stdio(saved_in, saved_out);
            } else {
                /* external command: find path and execute; execute_external_command will apply redir in child */
                char *cmdpath = path_search(tokens->items[0]);
                if (cmdpath) {
                    free(tokens->items[0]);
                    tokens->items[0] = cmdpath;
                    execute_external_command(tokens, &redir);
                } else {
                    printf("Command not found\n");
                }
            }
        }
        
        // for (int i = 0; i < tokens->size; i++) {
        //     printf("token %d: (%s)\n", i, tokens->items[i]);
        // }

        // tokens->items[0] = path_search(tokens->items[0]);
        // if (tokens->items[0]) {
        //     execute_external_command(tokens, &redir);
        // } else {
        //     printf("Command not found\n");
        // }
        
        
		free(input);
		free_tokens(tokens);
	}

	return 0;
}

tokenlist* environment_variable_expansion(tokenlist* tokens)
{

    for (int i = 0; i < tokens->size; i++) {
            
            if (tokens->items[i][0] == '$')
            {
                const char* var_name = &tokens->items[i][1]; // Skip the '$' character
                const char* var_value = getenv(var_name);
                if (var_value != NULL) {
                    // Replace the token with the environment variable value
                    free(tokens->items[i]); // Free the old token memory
                    tokens->items[i] = (char *)malloc(strlen(var_value) + 1); 
                    strcpy(tokens->items[i], var_value);
                } else {
                    // If the environment variable is not found, replace with an empty string
                    free(tokens->items[i]); // Free the old token memory
                    tokens->items[i] = (char *)malloc(1);
                    tokens->items[i][0] = '\0';
                }
            }
        }
        return tokens;
}

tokenlist* tilde_expansion(tokenlist* tokens)
{
    for (int i = 0; i < tokens->size; i++) {
        if (tokens->items[i][0] == '~' && (tokens->items[i][1] == '\0' || tokens->items[i][1] == '/')) {
            const char* home = getenv("HOME");
            if (home != NULL) {
                // If token is "~", rest_of_path is ""
                // If token is "~/something", rest_of_path is "/something"
                const char* rest_of_path = &tokens->items[i][1]; // points to "" or "/something"
                size_t new_length = strlen(home) + strlen(rest_of_path) + 1;
                char* full_path = (char *)malloc(new_length);
                strcpy(full_path, home);
                strcat(full_path, rest_of_path);
                free(tokens->items[i]);
                tokens->items[i] = full_path;
            } else {
                // If HOME is not found, replace with an empty string
                free(tokens->items[i]);
                tokens->items[i] = (char *)malloc(1);
                tokens->items[i][0] = '\0';
            }
        }
    }
    return tokens;
}

char* path_search(char* command)
{    
    if (command[0] == '/') {
        return NULL;
    }

    const char* path_env = getenv("PATH");
    
    // Create a copy of PATH since strtok modifies the string
    char* path_copy = (char*)malloc(strlen(path_env) + 1);

    strcpy(path_copy, path_env);
    
    char* dir = strtok(path_copy, ":");

    while (dir != NULL) {
        size_t full_path_length = strlen(dir) + 1 + strlen(command) + 1;
        char* full_path = (char*)malloc(full_path_length); 
        
        strcpy(full_path, dir);
        strcat(full_path, "/");
        strcat(full_path, command);

        if (access(full_path, X_OK) == 0) {
            free(path_copy);
            return full_path; // Found executable - caller must free this
        }
        
        free(full_path);
        dir = strtok(NULL, ":");
    }
    
    free(path_copy);
    return NULL; // Command not found in any PATH directory
}

void execute_external_command(tokenlist* tokens, const redir_t *r)
{
    pid_t pid = fork();
    if (pid == 0) {
               /* Child: apply redirection here (so parent is unaffected) */
        if (r && (r->in_file || r->out_file)) {
            if (apply_redirection(r, NULL, NULL) != 0) {
                _exit(1);
            }
        }

        /* build argv */
        char **argv = (char **)malloc((tokens->size + 1) * sizeof(char *));
        if (!argv) { _exit(1); }
        for (int i = 0; i < tokens->size; i++) {
            argv[i] = tokens->items[i];
        }
        argv[tokens->size] = NULL;

        execv(tokens->items[0], argv);
        perror("execv failed");
        free(argv);
        _exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("fork failed");
    }
}

void execute_pipeline(tokenlist **commands, int num_commands) {
    int pipes[2][2];
    pid_t pids[3];
    
    /* Create pipes */
    for (int i = 0; i < num_commands - 1; i++) {
        pipe(pipes[i]);
    }
    
    /* Fork and execute each command */
    for (int i = 0; i < num_commands; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            /* Child: setup pipes */
            if (i > 0) dup2(pipes[i-1][0], STDIN_FILENO);
            if (i < num_commands - 1) dup2(pipes[i][1], STDOUT_FILENO);
            
            /* Close all pipes */
            for (int j = 0; j < num_commands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            /* Build argv and execute */
            char **argv = malloc((commands[i]->size + 1) * sizeof(char*));
            for (int j = 0; j < commands[i]->size; j++) {
                argv[j] = commands[i]->items[j];
            }
            argv[commands[i]->size] = NULL;
            
            execv(commands[i]->items[0], argv);
            _exit(1);
        }
    }
    
    /* Parent: close pipes and wait */
    for (int i = 0; i < num_commands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    for (int i = 0; i < num_commands; i++) {
        waitpid(pids[i], NULL, 0);
    }
}

char *get_input(void) {
	char *buffer = NULL;
	int bufsize = 0;
	char line[5];
	while (fgets(line, 5, stdin) != NULL)
	{
		int addby = 0;
		char *newln = strchr(line, '\n');
		if (newln != NULL)
			addby = newln - line;
		else
			addby = 5 - 1;
		buffer = (char *)realloc(buffer, bufsize + addby);
		memcpy(&buffer[bufsize], line, addby);
		bufsize += addby;
		if (newln != NULL)
			break;
	}
	buffer = (char *)realloc(buffer, bufsize + 1);
	buffer[bufsize] = 0;
	return buffer;
}

tokenlist *new_tokenlist(void) {
	tokenlist *tokens = (tokenlist *)malloc(sizeof(tokenlist));
	tokens->size = 0;
	tokens->items = (char **)malloc(sizeof(char *));
	tokens->items[0] = NULL; /* make NULL terminated */
	return tokens;
}

void add_token(tokenlist *tokens, char *item) {
	int i = tokens->size;

	tokens->items = (char **)realloc(tokens->items, (i + 2) * sizeof(char *));
	tokens->items[i] = (char *)malloc(strlen(item) + 1);
	tokens->items[i + 1] = NULL;
	strcpy(tokens->items[i], item);

	tokens->size += 1;
}

tokenlist *get_tokens(char *input) {
	char *buf = (char *)malloc(strlen(input) + 1);
	strcpy(buf, input);
	tokenlist *tokens = new_tokenlist();
	char *tok = strtok(buf, " ");
	while (tok != NULL)
	{
		add_token(tokens, tok);
		tok = strtok(NULL, " ");
	}
	free(buf);
	return tokens;
}

void free_tokens(tokenlist *tokens) {
	for (int i = 0; i < tokens->size; i++)
		free(tokens->items[i]);
	free(tokens->items);
	free(tokens);
}


int parse_redirection(tokenlist *tokens, redir_t *r)
{
    r->in_file = NULL;
    r->out_file = NULL;
    r->append = 0;

    for (int i = 0; i < tokens->size; i++) {
        char *tok = tokens->items[i];
        if (strcmp(tok, "<") == 0 || strcmp(tok, ">") == 0 || strcmp(tok, ">>") == 0) {
            if (i + 1 >= tokens->size) {
                fprintf(stderr, "syntax error: expected filename after '%s'\n", tok);
                return -1;
            }
            char *fname = tokens->items[i + 1];

            if (strcmp(tok, "<") == 0) {
                /* set input file (replace previous if present) */
                free(r->in_file);
                r->in_file = xstrdup(fname);
            } else {
                free(r->out_file);
                r->out_file = xstrdup(fname);
                r->append = (strcmp(tok, ">>") == 0) ? 1 : 0;
            }

            /* remove the two tokens (operator and filename) from tokens list */
            free(tokens->items[i]);   /* free operator token memory */
            free(tokens->items[i + 1]); /* free filename token memory */
            for (int j = i + 2; j < tokens->size; j++) {
                tokens->items[j - 2] = tokens->items[j];
            }
            tokens->size -= 2;
            tokens->items = (char **)realloc(tokens->items, (tokens->size + 1) * sizeof(char *));
            tokens->items[tokens->size] = NULL;

            i--; /* re-check current index (it now has the next token) */
        }
    }
    return 0;
}

int apply_redirection(const redir_t *r, int *saved_stdin, int *saved_stdout)
{
    int in_fd = -1, out_fd = -1;

    /* Save current stdio fds so caller can restore */
    if (saved_stdin) {
        *saved_stdin = dup(STDIN_FILENO);
        if (*saved_stdin == -1) return -1;
    }
    if (saved_stdout) {
        *saved_stdout = dup(STDOUT_FILENO);
        if (*saved_stdout == -1) {
            if (saved_stdin) { close(*saved_stdin); *saved_stdin = -1; }
            return -1;
        }
    }

    if (r->in_file) {
        in_fd = open(r->in_file, O_RDONLY);
        if (in_fd < 0) {
            perror(r->in_file);
            return -1;
        }
        if (dup2(in_fd, STDIN_FILENO) == -1) {
            perror("dup2 stdin");
            close(in_fd);
            return -1;
        }
        close(in_fd);
    }

    if (r->out_file) {
        int flags = O_WRONLY | O_CREAT | (r->append ? O_APPEND : O_TRUNC);
        out_fd = open(r->out_file, flags, 0644);
        if (out_fd < 0) {
            perror(r->out_file);
            return -1;
        }
        if (dup2(out_fd, STDOUT_FILENO) == -1) {
            perror("dup2 stdout");
            close(out_fd);
            return -1;
        }
        close(out_fd);
    }

    return 0;
}

void restore_stdio(int saved_stdin, int saved_stdout)
{
    if (saved_stdin >= 0) {
        dup2(saved_stdin, STDIN_FILENO);
        close(saved_stdin);
    }
    if (saved_stdout >= 0) {
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
    }
}

/* simple builtin echo: tokens should have redirections already removed */
int builtin_echo(tokenlist *tokens)
{
    /* print tokens->items[1..] separated by spaces and newline */
    for (int i = 1; i < tokens->size; i++) {
        if (i > 1) putchar(' ');
        fputs(tokens->items[i], stdout);
    }
    putchar('\n');
    fflush(stdout);
    return 0;
}



