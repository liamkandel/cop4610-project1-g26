#include "pipeline.h"

pid_t execute_pipeline(tokenlist **commands, int num_commands, int background) {
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
    
    if (!background) {
        for (int i = 0; i < num_commands; i++) {
            waitpid(pids[i], NULL, 0);
        }
    } else {
        for (int i = 0; i < num_commands; i++) {
            waitpid(pids[i], NULL, WNOHANG);
        }
    }
    return pids[num_commands - 1];
}


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