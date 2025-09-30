#include "path_search.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
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
