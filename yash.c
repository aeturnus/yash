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
            if( !Tokenizer_hasTokens(tok) )
                return 0;
            replaceFile( STDOUT_FILENO, Tokenizer_next(tok), O_WRONLY | O_CREAT, RWURGRO );
        }
        else if ( !strcmp( token, "2>" ) )
        {
            if( !Tokenizer_hasTokens(tok) )
                return 0;
            replaceFile( STDERR_FILENO, Tokenizer_next(tok), O_WRONLY | O_CREAT, RWURGRO );
        }
        else if ( !strcmp( token, "<" ) )
        {
            if( !Tokenizer_hasTokens(tok) )
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
        //close(inPipe->fd[0]);               // we're done with this new fd
    }
    if(outPipe)
    {
        close(outPipe->fd[0]);                  // close off the read end
        dup2(outPipe->fd[1], STDOUT_FILENO);    // dup to stdout
        //close(outPipe->fd[1]);                  // we're done with this new fd
    }
}

void cleanPipes( Pipe *pipes, int numPipes  )
{
    for( int i = 0; i < numPipes; i++ )
    {
        close(pipes[i].fd[0]);
        close(pipes[i].fd[1]);
    }
}

void exec( Command *cmd, Pipe *pipes, int numPipes )
{
    char **argList;
    if( !parseCommand( cmd, &argList ) )
    {
        printf("Child failed to setup exec!\n");
        return 0;
    }
    if( pipes )
    {
        setupPipes( cmd );
        cleanPipes( pipes, numPipes );
    }
    //TODO: copy my environment variables
    execvp(argList[0], argList);
    free(argList);
}

pid_t forkexec( Command *cmd, Pipe *pipes, int numPipes )
{

    pid_t cpid = fork();
    // child do the work
    if(cpid == 0)
    {
        exec(cmd,pipes,numPipes);
    }

    return cpid;
}

// similar to forkexec, but starts a new group
pid_t forkexec_grp( Command **cmds, int numCmds )
{
    pid_t cpid = fork();
    if( cpid == 0 )
    {
        setsid();   // new session!
        // setup the pipes
        Pipe *pipes = malloc( sizeof(Pipe) * (numCmds-1) );
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

        // fork the children!
        for( int i = 1; i < numCmds; i++ )
        {
            forkexec( cmds[i], pipes, numCmds-1 );
        }
        // glorious leader gets his turn
        exec( cmds[0], pipes, numCmds-1 );

        /*
        // close off the pipes
        for( int i = 0; i < numCmds-1; i++)
        {
            close(pipes[i].fd[0]);
            close(pipes[i].fd[1]);
        }
        free(pipes);
        */
    }
    return cpid;
}

void addProc( Shell * shell, Process * proc )
{
    VVector_push( shell->procTable, proc );
}
void remProc( Shell * shell, Process * proc )
{
    VVector_remove( shell->procTable, proc );
}


void parseLine( Shell * shell )
{
    Tokenizer *tok = Tokenizer_new( shell->line, "|" ); // split it amongst pipes
    int numCmds = Tokenizer_numTokens( tok );           // get the number of tasks
    //ProcessState state = Tokenizer_contains(tok,"&")? bg: fg;   // figure this out from presence of &
    ProcessState state = strchr(shell->line,'&')? bg: fg;   // figure this out from presence of &
    pid_t cpid;
    if( numCmds == 1 )
    {
        Command cmd;
        Command_ctor( &cmd, Tokenizer_next(tok) );
        cpid = forkexec( &cmd, NULL, 0 );

        Process *child = Process_new( shell->line, cpid, state );
        addProc( shell, child );
        if( state == fg )
        {
            shell->active = child;
            waitpid(child->pid);
            remProc( shell, child );
        }
        else if ( state == bg )
        {
            printf("bg %d\n", child->pid);
        }

        Command_dtor( &cmd );
    }
    else if (numCmds > 1)
    {
        // need a list of commands and pipes
        Command **cmds = malloc( sizeof(Command) * numCmds );

        for( int i = 0; i < numCmds; i++)
        {
            cmds[i] = Command_new(Tokenizer_next(tok)); // new command
        }

        cpid = forkexec_grp(cmds, numCmds);

        waitpid(-cpid);
        for( int i = 0; i < numCmds; i++ )
        {
            free(cmds[i]);
        }
        free(cmds);
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
