#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <signal.h>

#include "shell.h"
#include "tok.h"

pid_t pid;

/**** Signal handlers ****/



/*************************/


/* prompts and reads a line into a buffer. very sad :( gets() */
void prompt( Shell *sh )
{
    // print prompt
    printf("%s", sh->prompt );
    // TODO: null check
    gets( sh->line );
}

// arglist must have already been allocated
char ** copyArgs( int argc, const char * const * argv, char *argList[] )
{
    argList = malloc(sizeof(char *) * argc + 1); // +1 for NULL
    for( int i = 0; i < argc; i++ )
    {
        argList[i] = strdup( argv[i] );
    }
    argList[argc] = NULL;
    return argList;
}

pid_t forkexec( int argc, const char * const * argv )
{
    pid_t cpid = fork();

    // child do the work
    if(cpid == 0)
    {
        //TODO: copy my environment variables
        char **argList;
        argList = copyArgs( argc, argv, argList );
        execvp(argList[0], argList);
    }

    return cpid;
}

void parse( Shell * shell )
{
    Tokenizer *tok = Tokenizer_new( shell->line, "|" ); // split it amongst pipes
    int numTasks = Tokenizer_numTokens( tok );          // get the number of tasks
    if( numTasks == 1 )
    {
        pid_t cpid;
        Tokenizer *tok1 = Tokenizer_new( Tokenizer_next(tok), " " );
        cpid = forkexec( Tokenizer_numTokens(tok1), Tokenizer_tokens(tok1) );
        waitpid(cpid);
    }
    else
    {

    }
}

int main( int argc, char *argv[] )
{
    pid = getpid();

    // setup shell struct
    Shell *yash = Shell_new();
    Shell_setPrompt( yash, "# " );

    // need to register handlers

    // execution loop
    // must read in lines, then parse them
    while(1)
    {
        prompt( yash );
        parse( yash );
    }
}
