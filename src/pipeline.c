#include "pipeline.h"

pid_t execute_pipeline(tokenlist **commands, int num_commands, int background) {
    int pipes[2][2]; // Max 2 pipes for 3 commands  
    pid_t pids[3];   // Max 3 processes
    
    // Create pipes (n-1 pipes for n commands)
    for (int i = 0; i < num_commands - 1; i++) {
        pipe(pipes[i]);
    }
    
    // Fork and execute each command
    for (int i = 0; i < num_commands; i++) {
        pids[i] = fork();
        
        if (pids[i] == 0) {
            // Child process
            
            // Connect input from previous pipe (except first command)
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            
            // Connect output to next pipe (except last command)
            if (i < num_commands - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            
            // Close all pipe file descriptors
            for (int j = 0; j < num_commands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            // Execute the command
            char **argv = malloc((commands[i]->size + 1) * sizeof(char*));
            for (int j = 0; j < commands[i]->size; j++) {
                argv[j] = commands[i]->items[j];
            }
            argv[commands[i]->size] = NULL;
            
            execv(commands[i]->items[0], argv);
            _exit(1);
        }
    }
    
    // Parent: close all pipes
    for (int i = 0; i < num_commands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    // Wait for processes
    if (!background) {
        for (int i = 0; i < num_commands; i++) {
            waitpid(pids[i], NULL, 0);
        }
    }
    
    // Return last process PID for background jobs
    return pids[num_commands - 1];
}


tokenlist** parse_pipes(tokenlist *tokens, int *num_commands) {
    *num_commands = 1;
    
    // Count number of commands by counting pipes
    for (int i = 0; i < tokens->size; i++) {
        if (strcmp(tokens->items[i], "|") == 0) {
            (*num_commands)++;
        }
    }
    
    // No pipes found
    if (*num_commands == 1) return NULL;
    
    // Allocate command arrays
    tokenlist **commands = malloc((*num_commands) * sizeof(tokenlist*));
    for (int i = 0; i < *num_commands; i++) {
        commands[i] = new_tokenlist();
    }
    
    // Split tokens into commands
    int current_cmd = 0;
    for (int i = 0; i < tokens->size; i++) {
        if (strcmp(tokens->items[i], "|") == 0) {
            current_cmd++;
        } else {
            add_token(commands[current_cmd], tokens->items[i]);
        }
    }
    
    return commands;
}
