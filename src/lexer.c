#include "lexer.h"
#include "expand.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

char *substitute_token(char *token, int is_command);

void execute_command(tokenlist *tokens) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: build argv array
        char **argv = malloc((tokens->size + 1) * sizeof(char *));
        for (int j = 0; j < tokens->size; j++) {
            argv[j] = tokens->items[j];
        }
        argv[tokens->size] = NULL;
        execv(argv[0], argv);
        perror("execv failed");
        exit(1);
    } else if (pid > 0) {
        // Parent process: wait for child
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("fork failed");
    }
}

int main()
{
    while (1) {
        show_prompt();

        /* input contains the whole command
         * tokens contains substrings from input split by spaces
         */

        char *input = get_input();
        printf("whole input: %s\n", input);

        tokenlist *tokens = get_tokens(input);

        tokens = expand_tokens(tokens);

        for (int i = 0; i < tokens->size; i++) {
			printf("token %d: (%s)\n", i, tokens->items[i]);
            char *newtok = substitute_token(tokens->items[i], i == 0);
            if (newtok != tokens->items[i]) {
                free(tokens->items[i]);
                tokens->items[i] = newtok;
            }
        }

        // Handle built-in 'cd' command
        if (tokens->size > 0 && strcmp(tokens->items[0], "cd") == 0) {
            // const char *target = tokens->size > 1 ? tokens->items[1] : getenv("HOME");
            // if (target == NULL) target = ".";
            // if (chdir(target) != 0) {
            //     perror("cd");
            // } else {
            //     // Update PWD and CWD environment variables after successful chdir
            //     char cwd[1024];
            //     if (getcwd(cwd, sizeof(cwd)) != NULL) {
            //         setenv("PWD", cwd, 1);
            //         setenv("CWD", cwd, 1);
            //     }
            // }
        } else if (tokens->size > 0 && strcmp(tokens->items[0], "echo") == 0) {
            // Implement echo internally
            for (int i = 1; i < tokens->size; i++) {
                printf("%s", tokens->items[i]);
                if (i < tokens->size - 1) printf(" ");
            }
            printf("\n");
        } else {
            execute_command(tokens);
        }

        free(input);
        free_tokens(tokens);
    }

    return 0;
}

void show_prompt() {
    char *cwd = getenv("PWD");
    char *user = getenv("USER");
    char *machine = getenv("MACHINE");

    printf("%s@%s:%s> ", user, machine, cwd);
    fflush(stdout);
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

char *substitute_token(char *token, int is_command) {
    // --- Environment variable expansion ($VAR) ---
    if (token[0] == '$' && strlen(token) > 1) {
        char *val = getenv(token + 1);
        if (val) {
            return strdup(val);
        }
        return strdup(""); // variable not found → empty string
    }

    if (token[0] == '~') {
        char *home = getenv("HOME");
        if (home) {
            char *expanded = malloc(strlen(home) + strlen(token));
            sprintf(expanded, "%s%s", home, token + 1);
            return expanded;
        }
    }

    if (is_command && strchr(token, '/') == NULL) {
        char *path = getenv("PATH");
        if (path) {
            char *pathcopy = strdup(path);
            char *dir = strtok(pathcopy, ":");
            while (dir != NULL) {
                char fullpath[1024];
                snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, token);
                if (access(fullpath, X_OK) == 0) {
                    free(pathcopy);
                    return strdup(fullpath);
                }
                dir = strtok(NULL, ":");
            }
            free(pathcopy);
        }
    }

    // No substitution performed → return original pointer
    return token;
}
