/* parse_string.h */

#ifndef PARSE_STRING_H_INCLUDED
#define PARSE_STRING_H_INCLUDED

#include "str.h"

error_code there_is_a_parsing_error(const string *str);
/*
    Checks  for parsing errors in processed string.
RECEIVES:
    - `str` address of `string` structure;
RETURNES:
    - error code indicating type of error or `no_error` */

void process_character(string *str);
/*
    Processes a single character during parsing.
RECEIVES:
    - `str` address of `string` structure; */

#endif
