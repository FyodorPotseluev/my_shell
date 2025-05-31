/* execute_command.h */

#ifndef EXECUTE_COMMAND_H_INCLUDED
#define EXECUTE_COMMAND_H_INCLUDED

#include "cmd_line.h"

void execute_command(cmd_line *cmdline);
/*
    Executes the command line with proper process management.
RECEIVES:
    - `cmdline` address of the `cmd_line` structure */

#endif
