#ifndef __SHELL_H__
#define __SHELL_H__

#include <stdlib.h>

#define CWD_SIZE 1000
#define PROMPT_SIZE 1000
#define LINE_SIZE 2000

typedef struct
{
    char *cwd;
    char *prompt;
    char *line;
} Shell;

typedef struct
{

} Task;

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

#endif //__SHELL_H__
