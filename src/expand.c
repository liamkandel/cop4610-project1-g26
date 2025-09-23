#include "expand.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

char* expand_token(char *token);
char* expand_env(char *token);

char *expand_env(char *token) {
    size_t tokenLen = strlen(token);
    int expandedLength = tokenLen;
    int beginEnv = -1, endEnv = -1;
    int added = 0, removed = 0;

    char *expanded = NULL;

    if (tokenLen <= 1) return token;

    for (int i = 0; i < tokenLen; i++) {
        if (token[i] == '$') beginEnv = i;
        if (beginEnv != -1 && !(isalnum(token[i + 1]) || token[i + 1] == '_')) {
            endEnv = i;
            size_t envVarLength = endEnv - beginEnv; 

            char tempEnv[envVarLength + 1];
            memcpy(tempEnv, token + beginEnv + 1, envVarLength);
            tempEnv[envVarLength] = '\0';

            char *env = getenv(tempEnv);
            if (env != NULL) {
                int oldOffset = expandedLength - tokenLen;
                char *oldExpanded = expanded;
                size_t expansionLength = strlen(env);
                expandedLength = tokenLen + added - removed + expansionLength;
                expanded = malloc(expandedLength + 1);

                char *debug = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

                // Copy the part before expansion
                if (oldExpanded == NULL) {
                    memcpy(expanded, token, beginEnv);
                } else {
                    memcpy(expanded, oldExpanded, beginEnv + added - removed);
                    expanded[beginEnv + added + 1] = '\0';
                    printf("Prefix: %s\nBeginEnv: %d\nOffset: %d", expanded, beginEnv, added); 
                    free(oldExpanded);
                }
                // Copy env expansion
                memcpy(expanded + added - removed + beginEnv, env, expansionLength);
                // Copy part after expansion
                memcpy(expanded + added - removed + expansionLength, token + endEnv + 1, tokenLen - endEnv + 1);

                expanded[expandedLength] = '\0';

                added += expansionLength;
                removed += envVarLength;
                
                printf("THE ENV EXPANDED!!: %s\n", expanded);
            } else {
                // This should do a check for assignment of custom env vars and if it's not an assignemnt it should errork
            }
            beginEnv = -1;
            endEnv = -1;
        }
    }

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
        char *expanded = malloc(strlen(home) + strlen(token));
        sprintf(expanded, "%s%s", home, token + 1);
        return expanded;
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