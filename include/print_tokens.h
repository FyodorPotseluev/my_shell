/* print_tokens.h */

#ifndef PTINT_TOKENS_H_INCLUDED
#define PTINT_TOKENS_H_INCLUDED

#include "str.h"

void print_tokens(const curr_str_words_list *words_list);
/*
    Prints all tokens of the linked list of words.
    Activated with the GCC key `D=PRINT_TOKENS_MODE`.
    Used to check the correctness of parsing string into tokens.
RECEIVES:
    - `words_list` address of the list of words (tokens) `str->words_list.first`
*/

#endif
