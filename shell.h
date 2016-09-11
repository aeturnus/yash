#ifndef __SHELL_H__
#define __SHELL_H__

#include <stdlib.h>

#include "tok.h"
#include "vvector.h"

#define CWD_SIZE 1000
#define PROMPT_SIZE 1000
#define LINE_SIZE 2000

typedef enum
{
    fg,
    sp,
    bg
} ProcessState;

typedef struct
{
    int fd[2];
} Pipe;

typedef struct
{
    Tokenizer *tok;     // tokenizer
    Pipe *inPipe;
    Pipe *outPipe;
} Command;

typedef struct
{
    pid_t pid;
    ProcessState state;
    char *command;
} Process;

typedef struct
{
    char *cwd;
    char *prompt;
    char *line;

    VVector *procTable;
    Process *active;
} Shell;

/**
 * Constructor
 * Will construct the shell object
 */
void Shell_ctor( Shell *this );

/**
 * Destructor
 * Will destroy the shell object
 */

void Shell_dtor( Shell *this );

Shell * Shell_new( void );
void Shell_delete( Shell **handle);

/**
 * setPrompt
 * Will set the shell's prompt
 * @param prompt The prompt string
 */
void Shell_setPrompt( Shell *this, const char *prompt );

void Command_ctor( Command *this, const char *line );
void Command_dtor( Command *this );
Command *Command_new( const char *line );
void Command_delete( Command *this );
void Process_ctor( Process *this, const char *line, pid_t pid, ProcessState state );
void Process_dtor( Process *this );
Process * Process_new( const char *line, pid_t pid, ProcessState state );
void Process_delete( Process *this );

#endif //__SHELL_H__
