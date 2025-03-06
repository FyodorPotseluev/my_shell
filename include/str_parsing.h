/* str_parsing.h */

#ifndef STR_PARSING_H_INCLUDED
#define STR_PARSING_H_INCLUDED

#include "constants.h"

void free_list_of_words(curr_str_words_list *link_list);

void process_end_of_string(string *str);

void process_character(string *str);

#endif
