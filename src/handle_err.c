/* handle_err.c */

#include "handle_err.h"
#include <errno.h>
#include <stdio.h>

static void print_error(const char *file_name, int line_num, const char *cmd)
{
    fprintf(stderr, "%s, %d, %s: ", file_name, line_num, cmd);
    perror("");
}

void pointer_error_handling(
    void *ptr, const char *file_name, int line_num, const char *cmd
)
{
    if (!ptr)
        print_error(file_name, line_num, cmd);
}

void error_handling(
    int res, const char *file_name, int line_num, const char *cmd
)
{
    if (res == -1) {
        print_error(file_name, line_num, cmd);
        fflush(stderr);
    }
}
