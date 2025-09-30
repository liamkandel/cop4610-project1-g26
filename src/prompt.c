#include "prompt.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
void print_prompt(void)
{
    const char* user = getenv("USER");
    const char* cwd = getcwd(NULL, 0);
    const char* machine = getenv("MACHINE");
    printf("%s@%s:%s$ ", user ? user : "", machine ? machine : "", cwd ? cwd : "");
    fflush(stdout);
}
