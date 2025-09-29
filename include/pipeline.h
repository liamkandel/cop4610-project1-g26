#include "lexer.h"
pid_t execute_pipeline(tokenlist **commands, int num_commands, int background);
int parse_pipes(tokenlist *tokens, tokenlist ***commands);