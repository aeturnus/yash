#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "shell.h"
#include "tok.h"
#include "vvector.h"

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


int replaceFile( int toClose, const char * filepath, int flags, mode_t mode )
{
    int status = 0;
    status = close(toClose);
    if( status != 0 )
        return 0;
    int fd = open(filepath, flags, mode);
    if( fd < 0 )
    {
        return 0;
    }

    /*
    dup(fd);    // duplicate it
    close(fd);
    */
    return 1;
}

#define RWURGRO ( S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH )
int setupExec( Tokenizer *tok, char **argList[] )
{
    const char * token;
    VVector *args = VVector_new(1);
    while( Tokenizer_hasTokens(tok) )
    {
        token = Tokenizer_next(tok);
        if( !strcmp( token, ">" ) )
        {
            if( !Tokenizer_hasTokens )
                return 0;
            replaceFile( STDOUT_FILENO, Tokenizer_next(tok), O_WRONLY | O_CREAT, RWURGRO );
        }
        else if ( !strcmp( token, "2>" ) )
        {
            if( !Tokenizer_hasTokens )
                return 0;
            replaceFile( STDERR_FILENO, Tokenizer_next(tok), O_WRONLY | O_CREAT, RWURGRO );
        }
        else if ( !strcmp( token, "<" ) )
        {
            if( !Tokenizer_hasTokens )
                return 0;
            replaceFile( STDIN_FILENO, Tokenizer_next(tok), O_RDONLY, RWURGRO );
        }
        else
        {
            VVector_push( args, token );
        }
    }
    // Set up the arglist
    int argc = VVector_length(args);
    char ** argv = malloc(sizeof(char *) * argc + 1);       // leave room for null
    for( int i = 0; i < argc; i++ )
    {
        argv[i] = VVector_get(args, i);    // set up the args
    }
    argv[argc] = NULL;    // null terminator

    // clean up!
    VVector_delete(args);

    *argList = argv;
    return 1;
}

pid_t forkexec( Tokenizer *tok )
{
    pid_t cpid = fork();

    // child do the work
    if(cpid == 0)
    {
        //TODO: copy my environment variables
        char **argList;
        if( !setupExec( tok, &argList ) )
        {
            printf("Child failed to setup exec!\n");
            exit(1);
        }
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
        cpid = forkexec( tok1 );
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
