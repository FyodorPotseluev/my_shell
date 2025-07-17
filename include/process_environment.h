/* process_environment.h */

#ifndef PROCESS_ENVIRONMENT_H_INCLUDED
#define PROCESS_ENVIRONMENT_H_INCLUDED

typedef struct tag_process_envir {
    int session_control_terminal_fd;
    int my_shell_group_pgid;
} type_process_envir;

void get_process_envir(type_process_envir *process_envir);

#endif
