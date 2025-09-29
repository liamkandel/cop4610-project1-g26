#include "lexer.h"

int parse_redirection(tokenlist *tokens, redir_t *r); /* removes redir tokens from tokens */
int apply_redirection(const redir_t *r, int *saved_stdin, int *saved_stdout); /* dup2s; returns 0 on success */
void restore_stdio(int saved_stdin, int saved_stdout);