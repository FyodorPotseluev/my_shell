/* cmd_execution.h */

#ifndef CMD_EXECUTION_H_INCLUDED
#define CMD_EXECUTION_H_INCLUDED

#include "constants.h"
#include <stdbool.h>

bool words_list_is_empty(const string *str);

void cleanup_background_zombies();

void execute_command(string *str);

#endif
