#include "expand.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

int min(int a, int b) {
    if (a < b) return a;
    else return b;
}

int max(int a, int b) {
    if (a > b) return a;
    else return b;
}

char* expand_token(char *token, int i);
char* expand_env(char *token);
char* addsubstr(char *a, char *b, int end_a, int end_b);
char* expand_path(char *token);

char* expand_path(char *token) {
    if (strchr(token, '/') == NULL) {
        char *path = getenv("PATH");
        if (path) {
            char *pathcopy = strdup(path);
            char *dir = strtok(pathcopy, ":");
            while (dir != NULL) {
                char fullpath[1024];
                snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, token);
                if (access(fullpath, X_OK) == 0) {
                    free(pathcopy);
                    return strdup(fullpath);
                }
                dir = strtok(NULL, ":");
            }
            free(pathcopy);
        }
    }
}

char* addsubstr(char *a, char *b, int end_a, int begin_b) {
    end_a = min(end_a, strlen(a) - 1);
    if (end_a == -1) end_a = strlen(a) - 1;
    if (begin_b < 0) begin_b = 0;

    char *res = malloc(end_a + 1 + strlen(b) - begin_b + 1);

    for (int i = 0; i <= end_a; i++) res[i] = a[i];
    for (int i = begin_b; i < strlen(b); i++) res[i + min(end_a, strlen(a)) - begin_b] = b[i];

    res[end_a + strlen(b) - 1 - begin_b + 1] = '\0';

    return res;
}

char* expand_env(char *token) {
    size_t tokenLen = strlen(token);
    int beginEnv = -1, endEnv = -1;

    if (tokenLen <= 1) return token;

    for (int i = 0; token[i] != '\0'; i++) {
        if (token[i] == '$') beginEnv = i;
        if (beginEnv != -1 && !(isalnum(token[i + 1]) || token[i + 1] == '_')) {
            endEnv = i;
            size_t envVarLength = endEnv - beginEnv; 

            char tempEnv[envVarLength + 1];
            memcpy(tempEnv, token + beginEnv + 1, envVarLength);
            tempEnv[envVarLength] = '\0';

            char *env = getenv(tempEnv);
            if (env != NULL) {
                char* expandedToken = addsubstr(token, env, beginEnv, -1);
                i = strlen(expandedToken) - 1;
                expandedToken = addsubstr(expandedToken, token, -1, endEnv + 1);

                free(token);
                token = expandedToken;

                // printf("Magic: %s\n", expandedToken);
                beginEnv = -1;
                endEnv = -1;
            } else {
                beginEnv = -1;
                endEnv = -1;
                // This should do a check for assignment of custom env vars and if it's not an assignemnt it should errork
            }
        }
    }

    char *env = getenv(token + 1);
    if (env == NULL) return token;

    token = strdup(env); 
    return token;
}

char *expand_token(char *token, int i) {

    // Honestly we should probably call expand_token after each expansion, to make sure the token is fully expanded..

    // Env var expansion
    token = expand_env(token);

    if (i == 0) token = expand_path(token);

    // Tilde expansion
    if (token[0] == '~') {
        char *home = getenv("HOME");
        char *expanded = malloc(strlen(home) + strlen(token));
        sprintf(expanded, "%s%s", home, token + 1);
        free(token);
        return expanded;
    }

    return token;
}

tokenlist *expand_tokens(tokenlist *tokens) {
    for (int i = 0; i < tokens->size; i++) {
        if (tokens->items[i] == NULL) continue; // Maybe we should handle empty NULL tokens as an error, since I don't think that's suppposed to happen
        tokens->items[i] = expand_token(tokens->items[i], i);
    }
    return tokens;
}