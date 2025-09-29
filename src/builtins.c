#include "builtins.h"

void builtin_jobs(jobs_t *jobs)
{
    if (!jobs) {
        printf("No active background processes\n");
        return;
    }
    int any = 0;
    for (int i = 0; i < 10; i++) {
        if (jobs->pids[i] != 0) {
            printf("[%d]+ %d %s\n", jobs->ids[i], (int)jobs->pids[i], jobs->cmds[i] ? jobs->cmds[i] : "");
            any = 1;
        }
    }
    if (!any) {
        printf("No active background processes\n");
    }
}


void builtin_exit(char** history, int history_count, pid_t* bg_pids, int bg_job_count) {
    // Wait for all background processes to finish
    if (bg_job_count)
    {
        printf("Waiting for background jobs to finish...\n");
    }
    for (int i = 0; i < bg_job_count; i++) {
        if (bg_pids[i] > 0) {
            waitpid(bg_pids[i], NULL, 0);
        }
    }

    // Display the last three valid commands
    printf("Last commands:\n");
    if (history_count == 0) {
        printf("No valid commands in history.\n");
    } else {
        for (int i = 0; i < (history_count > 3 ? 3 : history_count); i++) {
            printf("%s\n", history[i]);
        }
    }

    // Cleanup history
    for (int i = 0; i < history_count; i++) {
        free(history[i]);
    }

    exit(0);
}

int builtin_echo(tokenlist *tokens)
{
    // print tokens->items[1..] separated by spaces and newline 
    for (int i = 1; i < tokens->size; i++) {
        if (i > 1) putchar(' ');
        fputs(tokens->items[i], stdout);
    }
    putchar('\n');
    fflush(stdout);
    return 0;
}

int builtin_cd(tokenlist *tokens)
{
    char *target_dir = NULL;
    
    // Check number of arguments 
    if (tokens->size > 2) {
        fprintf(stderr, "cd: too many arguments\n");
        return -1;
    }
    
    // If no arguments provided, change to HOME 
    if (tokens->size == 1) {
        target_dir = getenv("HOME");
        if (!target_dir) {
            fprintf(stderr, "cd: HOME environment variable not set\n");
            return -1;
        }
    } else {
        // Use provided directory argument 
        target_dir = tokens->items[1];
    }
    
    // Attempt to change directory 
    if (chdir(target_dir) != 0) {
        perror("cd");
        return -1;
    }
    
    return 0;
}