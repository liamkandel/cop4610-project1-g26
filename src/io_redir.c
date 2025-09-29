#include "io_redir.h"
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
