/* process_environment.c */

#include "process_environment.h"
#include "handle_err.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void get_process_envir(type_process_envir *process_envir)
{
    process_envir->session_control_terminal_fd = open("/dev/tty", O_RDONLY);
    error_handling(
        process_envir->session_control_terminal_fd, __FILE__, __LINE__, "open"
    );
    process_envir->my_shell_group_pgid = getpgrp();
}
