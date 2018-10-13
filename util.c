#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

// Yes i really want to do this. _exit must not have the noreturn attribute.
//#include <unistd.h>
int _exit(int);

void fatal(int exitcode, char *fmt, ...){
    va_list args; 
    va_start(args, fmt);
    fflush(stdout);
    vfprintf(stderr, fmt, args);
    va_end(args);
    _exit(exitcode);
}
