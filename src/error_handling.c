#include "error_handling.h"
#include <stdio.h>

void error_handling(
    int res, const char *file_name, int line_num, const char *cmd
)
{
    if (res == -1) {
        fprintf(stderr, "%s, %d, %s: ", file_name, line_num, cmd);
        perror("");
        fflush(stderr);
    }
}
