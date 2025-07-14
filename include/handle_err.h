/* handle_err.h */

#ifndef HANDLE_ERR_H_INCLUDED
#define HANDLE_ERR_H_INCLUDED

void error_handling(
    int res, const char *file_name, int line_num, const char *cmd
);
/*  Handles system call errors with detailed reporting.
RECEIVES:
    - `res` return value from system call;
    - `file_name` source file name for error location;
    - `line_num` line number for error location;
    - `cmd` system call name that failed */

void pointer_error_handling(
    void *ptr, const char *file_name, int line_num, const char *cmd
);
/* Works exactly as the previous function, except it prints an error message if
it was passed the NULL `ptr` value. */

#endif
