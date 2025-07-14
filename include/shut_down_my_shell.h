/* shut_down_my_shell.h */

#ifndef SHUT_DOWN_MY_SHELL_H_INCLUDED
#define SHUT_DOWN_MY_SHELL_H_INCLUDED

#include "input.h"
#include "ternary_search_tree.h"
#include "tokenizer.h"

void shut_down_my_shell(
    type_input *input, type_tokenizer *tknzer, type_tst *path_tree
);
/* Handles the program termination and frees the memory used by the arguments it
receives */

#endif
