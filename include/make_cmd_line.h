/* make_cmd_line.h */

#ifndef MAKE_CMD_LINE_H_INCLUDED
#define MAKE_CMD_LINE_H_INCLUDED

#include "cmd_line.h"
#include "err_code.h"
#include "str.h"

error_code make_cmd_line(cmd_line *cmdline, string *str);
/*
    Converts parsed string tokens into executable command line structure.
RECEIVES:
    - `cmdline` address of `cmd_line` structure (output);
    - `str` address of parsed `string` structure (input);
RETURNES:
    - error code indicating success (`no_error`) or type of error */

#endif
