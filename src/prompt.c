#include "prompt.h"
#include <stdlib.h>
#include <stdio.h>
void print_prompt(void)
{
    const char* user = getenv("USER");
    const char* cwd = getenv("PWD");
    const char* machine = getenv("MACHINE");
    printf("%s@%s:%s$ ", user ? user : "", machine ? machine : "", cwd ? cwd : "");
    fflush(stdout);
}