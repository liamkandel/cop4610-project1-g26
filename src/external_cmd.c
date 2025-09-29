#include "external_cmd.h"
#include "io_redir.h"
pid_t execute_external_command(tokenlist* tokens, const redir_t *r, int background)
{
    pid_t pid = fork();
    if (pid == 0) {
               // Child: apply redirection here (so parent is unaffected) 
        if (r && (r->in_file || r->out_file)) {
            if (apply_redirection(r, NULL, NULL) != 0) {
                _exit(1);
            }
        }

        // build argv 
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
        if (!background) {
            int status;
            waitpid(pid, &status, 0);
        } else {
            waitpid(pid, NULL, WNOHANG);
        }
        return pid;
    } else {
        perror("fork failed");
        return -1;
    }
}