#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <signal.h>

#include "shell.h"

pid_t pid;


void readLine( Shell *shell )
{
    // TODO: null check
    gets( shell->line );
}

int main( int argc, char *argv[] )
{
    pid = getpid();

    Shell yash;
    Shell_ctor( &yash );
    getcwd( yash.cwd, CWD_SIZE );


    printf("My cwd: %s\n", yash.cwd);
    readLine( &yash );
    printf("Entered: %s\n", yash.line);

}
