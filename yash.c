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
#include "debug.h"

// must-have globals
pid_t pid;
Shell *shell;


/* prompts and reads a line into a buffer. very sad :( gets() */
void prompt( Shell *sh )
{
    // print prompt
    printf("%s", sh->prompt );
}

void readLine( Shell *sh )
{
    size_t pos = 0;
    char* buffer = sh->line;
    char c;
    while(1)
    {
        //Read character
        c = getchar();

        if(c == '\n')
        {
            buffer[pos] = '\0';
            return;
        }
        else if (c == EOF )
        {
            exit(0);
        }
        else if (c == "\027")   //escape character detection
        {

        }
        else
        {
            buffer[pos] = c;
        }
        pos++;
        /*
        //Reallocate if bigger than buffer size
        if(pos >= buffSize)
        {
            size_t newSize = buffSize * 2;
            char* tempBuffer = realloc(buffer, buffSize);
            if(tempBuffer == 0) // if realloc didn't have enough space
            {
                tempBuffer = malloc(newSize);
                memcpy(tempBuffer,buffer,pos-1);
                free(buffer);
            }
            buffer = tempBuffer;    //set buffer to equal the new buffer
        }
        */
    }
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
    exit( 0 );
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
        setpgid(0,0);
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

void addJob( Shell * shell, Job * job )
{
    VVector_push( shell->jobTable, job );
}
void remJob( Shell * shell, Job * job )
{
    VVector_remove( shell->jobTable, job );
}
void pushSusp( Shell * shell, Job * job)
{
    VVector_push( shell->suspStack, job );
}
void popSusp( Shell * shell, Job ** jobHandle)
{
    *jobHandle = VVector_pop( shell->suspStack );
}

void waitActive( Shell *shell );
// handle builtins:
// fg, bg, jobs
int parseBuiltin( Shell * shell )
{
    if( !strcmp(shell->line, "fg" ) )
    {
        popSusp( shell, &shell->active );
        if( shell->active )
        {
            shell->active->state = fg;
            printf("fg %d\n", shell->active->pid);
            kill( shell->active->pid, SIGCONT );
            waitActive(shell);
        }
        else
        {
            printf("No suspended jobs\n");
        }
        return 1;
    }
    else if ( !strcmp(shell->line, "bg") )
    {
        return 1;
    }
    return 0;
}

void parseLine( Shell * shell )
{
    if( parseBuiltin(shell) )
        return;

    Tokenizer *tok = Tokenizer_new( shell->line, "|" ); // split it amongst pipes
    int numCmds = Tokenizer_numTokens( tok );           // get the number of tasks
    //JobState state = Tokenizer_contains(tok,"&")? bg: fg;   // figure this out from presence of &
    JobState state = strchr(shell->line,'&')? bg: fg;   // figure this out from presence of &
    pid_t cpid;
    if( numCmds == 1 )
    {

        Tokenizer *cmdtok = Tokenizer_new(Tokenizer_next(tok), "&");

        Command cmd;
        Command_ctor( &cmd, Tokenizer_next(cmdtok) );
        cpid = forkexec( &cmd, NULL, 0 );

        Job *child = Job_new( shell->line, cpid, state );
        addJob( shell, child );
        if( state == fg )
        {
            shell->active = child;
            waitpid(child->pid);

        }
        else if ( state == bg )
        {
            // let it run
            printf("bg %d\n", child->pid);
        }

        Command_dtor( &cmd );
        Tokenizer_delete( cmdtok );
    }
    else if (numCmds > 1)
    {
        // need a list of commands and pipes
        Command **cmds = malloc( sizeof(Command *) * numCmds );

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

/**** Signal handlers ****/
void sigtstp_handler(int signo)
{
    //dprintf("SIGSTP received from terminal\n");
    prompt(shell);
    if( !shell->active )
    {
        //printf("\n");
        return;
    }

    // suspend active
    kill( shell->active->pid, SIGTSTP );
    //printf("Suspended %s (%d)\n", active->command, active->pid);
}

void sigint_handler(int signo)
{
    //dprintf("SIGINT received from terminal\n");
    prompt(shell);
    if( !shell->active )
    {
        //printf("\n");
        return;
    }
    // interrupt active
    kill( shell->active->pid, SIGINT );
}


/*************************/

void waitActive( Shell *shell )
{
    int wstatus;
    int wpid;
    if( shell->active )
    {
        dprintf("Waiting on %d \"%s\"\n", shell->active->pid, shell->active->command);
        wpid = waitpid( shell->active->pid, &wstatus, WUNTRACED | WCONTINUED );
        if( WIFEXITED(wstatus) )
        {
            dprintf("%d exited\n", wpid);
            remJob( shell, shell->active );
            shell->active = NULL;
        }
        else if ( WIFSTOPPED(wstatus) )
        {
            dprintf("%d stopped\n", wpid);
            shell->active->state = sp;              // suspend it
            pushSusp(shell, shell->active);  // push it onto the susp stack
            shell->active = NULL;
        }
        else if ( WIFCONTINUED(wstatus) )
        {
            dprintf("%d continued\n", wpid);
        }
    }
    else
    {
        //dprintf("No fg process\n");
    }
}

int main( int argc, char *argv[] )
{
    // setup shell struct
    Shell *yash = Shell_new();
    Shell_setPrompt( yash, "# " );

    // initialize globals
    shell = yash;
    pid = getpid();

    // need to register handlers
    signal( SIGTSTP, sigtstp_handler );
    signal( SIGINT, sigint_handler );

    // execution loop
    // must read in lines, then parse them
    while(1)
    {
        prompt( yash );
        readLine( yash );
        parseLine( yash );
        waitActive( yash );
    }
}
