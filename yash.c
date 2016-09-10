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
int parseCommand( Command *cmd, char **argList[] )
{

    Tokenizer *tok = cmd->tok;
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
    char ** argv = malloc(sizeof(char *) * (argc + 1) );       // leave room for null
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

void setupPipes( Command *cmd )
{
    Pipe *inPipe = cmd->inPipe;
    Pipe *outPipe = cmd->outPipe;
    if(inPipe)
    {
        close(inPipe->fd[1]);               // close off the write end
        dup2(inPipe->fd[0], STDIN_FILENO);  // dup to stdin
        close(inPipe->fd[0]);               // we're done with this new fd
    }
    if(outPipe)
    {
        close(outPipe->fd[0]);                  // close off the read end
        dup2(outPipe->fd[1], STDOUT_FILENO);    // dup to stdout
        close(outPipe->fd[1]);                  // we're done with this new fd
    }
}

pid_t forkexec( Command *cmd )
{

    pid_t cpid = fork();
    // child do the work
    if(cpid == 0)
    {
        char **argList;
        if( !parseCommand( cmd, &argList ) )
        {
            printf("Child failed to setup exec!\n");
            return 0;
        }

        setupPipes( cmd );
        //TODO: copy my environment variables
        execvp(argList[0], argList);
        free(argList);
    }

    return cpid;
}

void parseLine( Shell * shell )
{
    Tokenizer *tok = Tokenizer_new( shell->line, "|" ); // split it amongst pipes
    int numCmds = Tokenizer_numTokens( tok );          // get the number of tasks
    if( numCmds == 1 )
    {
        pid_t cpid;
        Command cmd;
        Command_ctor( &cmd, Tokenizer_next(tok) );
        cpid = forkexec( &cmd );
        waitpid(cpid);
        Command_dtor( &cmd );
    }
    else if (numCmds > 1)
    {
        // need a list of commands and pipes
        Command **cmds = malloc( sizeof(Command) * numCmds );
        Pipe *pipes = malloc( sizeof(Pipe) * (numCmds-1) );
        pid_t *cpids = malloc( sizeof(pid_t) * numCmds );

        // TODO: smarter way of pipe creation and close in parent
        for( int i = 0; i < numCmds; i++)
        {
            cmds[i] = Command_new(Tokenizer_next(tok)); // new command
        }
        for( int i = 0; i < numCmds-1; i++)
        {
            pipe(pipes[i].fd);
        }

        // Connect the pipes
        cmds[0]->inPipe = NULL;             // first process pipe
        cmds[0]->outPipe = &pipes[0];
        for( int i = 1; i < numCmds-1; i++ )
        {
            cmds[i]->inPipe = &pipes[i-1];
            cmds[i]->outPipe = &pipes[i];
        }
        cmds[numCmds-1]->inPipe = &pipes[numCmds-2];  // second process pipe
        cmds[numCmds-1]->outPipe = NULL;

        // fork and execute
        for( int i = 0; i < numCmds; i++)
        {
            cpids[i] = forkexec(cmds[i]);
        }

        // delete commands, shut down the pipes
        for( int i = 0; i < numCmds; i++)
        {
            waitpid(cpids[i]);
            Command_delete(cmds[i]);
        }
        for( int i = 0; i < numCmds-1; i++)
        {
            close(pipes[i]
        }
        free(cmds);
        free(pipes);
    }
    Tokenizer_delete(tok);
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
        parseLine( yash );
    }
}
