#include "bg_jobs.h"
jobs_t* jobs_init(jobs_t *j) {
    if (j) return j;
    j = (jobs_t*)calloc(1, sizeof(jobs_t));
    if (!j) return NULL;
    j->next_id = 1;
    for (int i = 0; i < 10; i++) { j->pids[i] = 0; j->cmds[i] = NULL; j->ids[i] = 0; }
    return j;
}

int jobs_add(jobs_t *j, pid_t pid, const char *cmdline) {
    if (!j || pid <= 0) return -1;
    for (int i = 0; i < 10; i++) {
        if (j->pids[i] == 0) {
            j->pids[i] = pid;
            j->ids[i] = j->next_id++;
            const char *cmd = cmdline ? cmdline : "";
            j->cmds[i] = malloc(strlen(cmd) + 1);
            strcpy(j->cmds[i], cmd);
            return j->ids[i];
        }
    }
    return -1;
}

void jobs_check(jobs_t *j) {
    if (!j) return;
    for (int i = 0; i < 10; i++) {
        if (j->pids[i] == 0) continue;
        int status = 0;
        pid_t r = waitpid(j->pids[i], &status, WNOHANG);
        if (r == 0) continue;
        if (r == j->pids[i] || (r == -1 && errno == ECHILD)) {
            printf("[%d] + done %s\n", j->ids[i], j->cmds[i] ? j->cmds[i] : "");
            free(j->cmds[i]);
            j->cmds[i] = NULL;
            j->pids[i] = 0;
            j->ids[i] = 0;
        }
    }
    while (waitpid(-1, NULL, WNOHANG) > 0) {}
}