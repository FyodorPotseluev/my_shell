/* make_cmd_line.h */

#ifndef MAKE_CMD_LINE_H_INCLUDED
#define MAKE_CMD_LINE_H_INCLUDED

#include "cmd_line.h"
#include "err_code.h"
#include "tokenizer.h"

error_code make_cmd_line(type_cmd_line *cmdline, type_tokenizer *tknzer);
/*
    Converts parsed input string tokens into executable command line structure.
RECEIVES:
    - `cmdline` address of `type_cmd_line` structure (output);
    - `tknzer` address of parsed `type_tokenizer` structure (input);
RETURNES:
    - error code indicating success (`no_error`) or type of error */

#endif
