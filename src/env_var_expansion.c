#include "env_var_expansion.h"

tokenlist* environment_variable_expansion(tokenlist* tokens)
{

    for (int i = 0; i < tokens->size; i++) {
            
            if (tokens->items[i][0] == '$')
            {
                const char* var_name = &tokens->items[i][1]; // Skip the '$' character
                const char* var_value = getenv(var_name);
                if (var_value != NULL) {
                    // Replace the token with the environment variable value
                    free(tokens->items[i]); // Free the old token memory
                    tokens->items[i] = (char *)malloc(strlen(var_value) + 1); 
                    strcpy(tokens->items[i], var_value);
                } else {
                    // If the environment variable is not found, replace with an empty string
                    free(tokens->items[i]); // Free the old token memory
                    tokens->items[i] = (char *)malloc(1);
                    tokens->items[i][0] = '\0';
                }
            }
        }
        return tokens;
}
