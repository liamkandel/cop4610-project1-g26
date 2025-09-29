#include "lexer.h"
#include <stdlib.h>
#include <stdio.h>

int builtin_echo(tokenlist *tokens);
int builtin_cd(tokenlist *tokens);
void builtin_jobs(jobs_t *jobs);
void builtin_exit(char **history, int history_count, pid_t *bg_pids, int bg_job_count);