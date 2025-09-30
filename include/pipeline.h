#include "lexer.h"
pid_t execute_pipeline(tokenlist **commands, int num_commands, int background);
tokenlist **parse_pipes(tokenlist *tokens, int *num_commands);
