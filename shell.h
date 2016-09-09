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

/**
 * Constructor
 * Will construct the shell object
 */
Shell_ctor( Shell *this );

/**
 * Constructor
 * Will destroy the shell object
 */

Shell_dtor( Shell *this );

#endif //__SHELL_H__
