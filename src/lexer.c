#include "lexer.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

tokenlist* environment_variable_expansion(tokenlist*tokens); // --- PART 2: ENVIRONMENT VARIABLE EXPANSION ---
tokenlist* tilde_expansion(tokenlist* tokens); // --- PART 3: TILDE EXPANSION ---
// implement path search function
char* path_search(char* tokens); // --- PART 4: PATH SEARCH ---

int main()
{
	while (1) {
		
        // ---- PART 1: PROMPT -----
        const char* user = getenv("USER");
        const char* cwd = getenv("PWD");
        const char* machine = getenv("MACHINE");

        printf("%s@%s:%s$ ", user, machine, cwd);


		/* input contains the whole command
		 * tokens contains substrings from input split by spaces
		 */

		char *input = get_input();
		printf("whole input: %s\n", input);


        tokenlist *tokens = get_tokens(input);
        tokens = environment_variable_expansion(tokens);
        tokens = tilde_expansion(tokens);

        

        tokens->items[0] = path_search(tokens->items[0]);
        for (int i = 0; i < tokens->size; i++) {

            printf("token %d: (%s)\n", i, tokens->items[i]);
		}

		free(input);
		free_tokens(tokens);
	}

	return 0;
}

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

char* path_search(char* command)
{    
    if (command[0] == '/') {
        return NULL;
    }

    const char* path_env = getenv("PATH");
    
    // Create a copy of PATH since strtok modifies the string
    char* path_copy = (char*)malloc(strlen(path_env) + 1);

    strcpy(path_copy, path_env);
    
    char* dir = strtok(path_copy, ":");

    while (dir != NULL) {
        size_t full_path_length = strlen(dir) + 1 + strlen(command) + 1;
        char* full_path = (char*)malloc(full_path_length); 
        
        strcpy(full_path, dir);
        strcat(full_path, "/");
        strcat(full_path, command);

        if (access(full_path, X_OK) == 0) {
            free(path_copy);
            return full_path; // Found executable - caller must free this
        }
        
        free(full_path);
        dir = strtok(NULL, ":");
    }
    
    free(path_copy);
    return NULL; // Command not found in any PATH directory
}


char *get_input(void) {
	char *buffer = NULL;
	int bufsize = 0;
	char line[5];
	while (fgets(line, 5, stdin) != NULL)
	{
		int addby = 0;
		char *newln = strchr(line, '\n');
		if (newln != NULL)
			addby = newln - line;
		else
			addby = 5 - 1;
		buffer = (char *)realloc(buffer, bufsize + addby);
		memcpy(&buffer[bufsize], line, addby);
		bufsize += addby;
		if (newln != NULL)
			break;
	}
	buffer = (char *)realloc(buffer, bufsize + 1);
	buffer[bufsize] = 0;
	return buffer;
}

tokenlist *new_tokenlist(void) {
	tokenlist *tokens = (tokenlist *)malloc(sizeof(tokenlist));
	tokens->size = 0;
	tokens->items = (char **)malloc(sizeof(char *));
	tokens->items[0] = NULL; /* make NULL terminated */
	return tokens;
}

void add_token(tokenlist *tokens, char *item) {
	int i = tokens->size;

	tokens->items = (char **)realloc(tokens->items, (i + 2) * sizeof(char *));
	tokens->items[i] = (char *)malloc(strlen(item) + 1);
	tokens->items[i + 1] = NULL;
	strcpy(tokens->items[i], item);

	tokens->size += 1;
}

tokenlist *get_tokens(char *input) {
	char *buf = (char *)malloc(strlen(input) + 1);
	strcpy(buf, input);
	tokenlist *tokens = new_tokenlist();
	char *tok = strtok(buf, " ");
	while (tok != NULL)
	{
		add_token(tokens, tok);
		tok = strtok(NULL, " ");
	}
	free(buf);
	return tokens;
}

void free_tokens(tokenlist *tokens) {
	for (int i = 0; i < tokens->size; i++)
		free(tokens->items[i]);
	free(tokens->items);
	free(tokens);
}
