#include "lexer.h"
jobs_t* jobs_init(jobs_t *j);
int jobs_add(jobs_t *j, pid_t pid, const char *cmdline);
void jobs_check(jobs_t *j);