#include "shell.h"
#include "tok.h"
#include "vvector.h"
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

    this->procTable = VVector_new_reg( 1, &Process_delete );
    this->suspStack = VVector_new( 1 );

    this->active = NULL;
}

Shell_dtor( Shell *this )
{
    free(this->cwd);
    this->cwd = NULL;

    free(this->prompt);
    this->prompt = NULL;

    free(this->line);
    this->line = NULL;

    VVector_deleteFull(this->procTable);
    this->procTable = NULL;

    VVector_delete(this->suspStack);
    this->suspStack = NULL;

    this->active = NULL;
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

void Shell_setPrompt( Shell *this, const char *prompt )
{
    strcpy( this->prompt, prompt );
}

void Command_ctor( Command *this, const char *line )
{
    this->tok = Tokenizer_new(line, " ");
    this->inPipe = this->outPipe = NULL;
}

void Command_dtor( Command *this )
{
    Tokenizer_delete( this->tok );
    this->inPipe = this->outPipe = NULL;
}

Command *Command_new( const char *line )
{
    Command *out = malloc( sizeof(Command) );
    Command_ctor( out, line );
    return out;
}

void Command_delete( Command *this )
{
    if( !this )
        return
    Command_dtor( this );
    free( this );
}

void Process_ctor( Process *this, const char * line, pid_t pid, ProcessState state )
{
    this->pid = pid;
    this->state = state;
    this->command = strdup(line);
}

void Process_dtor( Process *this )
{
    free(this->command);
}

Process * Process_new( const char * line, pid_t pid, ProcessState state )
{
    Process * out = malloc(sizeof(Process));
    Process_ctor(out, line, pid, state);
    return out;
}

void Process_delete( Process *this )
{
    Process_dtor(this);
    free(this);
}
