/* cmd_execution.h */

#ifndef CMD_EXECUTION_H_INCLUDED
#define CMD_EXECUTION_H_INCLUDED

#include "constants.h"
#include <stdbool.h>

void reset_cmd_line_item(execvp_cmd_line *item);

void init_cmd_line_item(execvp_cmd_line *item);

bool words_list_is_empty(const string *str);

void execute_command(string *str);

#endif
