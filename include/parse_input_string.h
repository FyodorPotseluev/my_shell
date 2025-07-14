/* parse_input_string.h */

#ifndef PARSE_INPUT_STRING_H_INCLUDED
#define PARSE_INPUT_STRING_H_INCLUDED

#include "err_code.h"
#include "input.h"
#include "tokenizer.h"

error_code there_is_a_parsing_error(const type_tokenizer *tknzer);
/*
    Checks  for parsing errors in processed string.
RECEIVES:
    - `tknzer` address of `type_tokenizer` structure;
RETURNES:
    - error code indicating type of error or `no_error` */

void parse_next_character(type_tokenizer *tknzer, type_input *input);
/*
    Processes a single character during parsing.
RECEIVES:
    - `tknzer` address of `type_tokenizer` structure; */

#endif
