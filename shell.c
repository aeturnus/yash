#include "shell.h"

#include <stdlib.h>
#include <string.h>

Shell_ctor( Shell *this )
{
    this->cwd = malloc( CWD_SIZE * sizeof(char) + 1 );
    this->cwd[0] = '\0';

    this->prompt = malloc( PROMPT_SIZE * sizeof(char) + 1 );
    this->prompt[0] = '\0';

    this->line = malloc( LINE_SIZE * sizeof(char) + 1 );
    this->line[0] = '\0';
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

Shell * Shell_new( void )
{
    Shell *out = malloc(sizeof(Shell));
    Shell_ctor(out);
    return out;
}

void Shell_delete( Shell **handle )
{
    Shell_dtor(*handle);
    free(*handle);
    handle = NULL;
}

Shell_setPrompt( Shell *this, const char *prompt )
{
    strcpy( this->prompt, prompt );
}
