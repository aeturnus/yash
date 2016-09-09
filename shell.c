#include "shell.h"

#include <stdlib.h>

Shell_ctor( Shell *this )
{
    this->cwd = malloc( CWD_SIZE * sizeof(char) );
    this->prompt = malloc( PROMPT_SIZE * sizeof(char) );
    this->line = malloc( LINE_SIZE * sizeof(char) );

}

Shell_dtor( Shell *this )
{
    free(this->cwd);
    this->cwd = NULL;

    free(this->prompt);
    this->prompt = NULL;

    free(this->line);
    this->line = NULL;
}
