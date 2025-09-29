#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

typedef struct {
    char ** items;
    size_t size;
} tokenlist;

typedef struct {
    pid_t pids[10];
    char *cmds[10];
    int ids[10];
    int next_id;
} jobs_t;

char * get_input(void);
tokenlist * get_tokens(char *input);
tokenlist * new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);

/* Forward declaration needed for execute_external_command and execute_pipeline */
typedef struct {
    char *in_file;
    char *out_file;
    int append;
} redir_t;

pid_t execute_external_command(tokenlist* tokens, const redir_t *r, int background);
pid_t execute_pipeline(tokenlist **commands, int num_commands, int background);

