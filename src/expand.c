#include "expand.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* expand_token(char *token);
char* expand_env(char *token);

char *expand_env(char *token) {
    if (strlen(token) <= 1 || token[0] != '$') return token;

    char *env = getenv(token + 1);
    if (env == NULL) return token;

    token = strdup(env); 
    return token;
}

char *expand_token(char *token) {

    // Honestly we should probably call expand_token after each expansion, to make sure the token is fully expanded..

    char *oldtoken = token;

    // Env var expansion
    token = expand_env(token);

    // Tilde expansion
    if (token[0] == '~') {
        char *home = getenv("HOME");
        if (home) token = strdup(home);
    }

    if (oldtoken != token) free(oldtoken);

    return token;
}

tokenlist *expand_tokens(tokenlist *tokens) {
    for (int i = 0; i < tokens->size; i++) {
        if (tokens->items[i] == NULL) continue; // Maybe we should handle empty NULL tokens as an error, since I don't think that's suppposed to happen
        tokens->items[i] = expand_token(tokens->items[i]);
    }
    return tokens;
}