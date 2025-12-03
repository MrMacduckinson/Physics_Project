#include <stdio.h>
#include <stdlib.h>

void __eprintf(const char *string, const char *expression,
               unsigned int line, const char *filename)
{
    fprintf(stderr, string, expression, line, filename);
    fflush(stderr);
    abort();
}
