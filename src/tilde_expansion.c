#include "tilde_expansion.h"

tokenlist* tilde_expansion(tokenlist* tokens)
{
    for (int i = 0; i < tokens->size; i++) {
        if (tokens->items[i][0] == '~' && (tokens->items[i][1] == '\0' || tokens->items[i][1] == '/')) {
            const char* home = getenv("HOME");
            if (home != NULL) {
                // If token is "~", rest_of_path is ""
                // If token is "~/something", rest_of_path is "/something"
                const char* rest_of_path = &tokens->items[i][1]; // points to "" or "/something"
                size_t new_length = strlen(home) + strlen(rest_of_path) + 1;
                char* full_path = (char *)malloc(new_length);
                strcpy(full_path, home);
                strcat(full_path, rest_of_path);
                free(tokens->items[i]);
                tokens->items[i] = full_path;
            } else {
                // If HOME is not found, replace with an empty string
                free(tokens->items[i]);
                tokens->items[i] = (char *)malloc(1);
                tokens->items[i][0] = '\0';
            }
        }
    }
    return tokens;
}