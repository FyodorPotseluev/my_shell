/* tokenizer.h */

#ifndef TOKENIZER_H_INCLUDED
#define TOKENIZER_H_INCLUDED

#include "err_code.h"
#include <stdbool.h>

enum str_h_consts {
    init_tmp_wrd_arr_len = 16
};

typedef enum tag_separator_type {
    none,
    background_operator,        /* &  */
    and_operator,               /* && */
    output_redirection,         /* >  */
    output_append_redirection,  /* >> */
    pipe_operator,              /* |  */
    or_operator,                /* || */
    input_redirection,          /* <  */
    command_separator,          /* ;  */
    open_parenthesis,           /* (  */
    close_parenthesis           /* )  */
} separator_type;

typedef struct tag_word_item {
    char *word;
    separator_type separator_val;
    struct tag_word_item *next;
} word_item;

typedef struct tag_curr_str_words_list {
    word_item *first;
    word_item *last;
} curr_str_words_list;

typedef struct tag_curr_word_dynamic_char_arr {
    char *arr;
    int idx;
    int arr_len;
} curr_word_dynamic_char_arr;

typedef struct struct_tokenizer {
    bool word_ended, str_ended, quotation, char_escaping;
    int c;
    error_code err_code;
    /* contains the array in which the current word is formed */
    curr_word_dynamic_char_arr tmp_wrd;
    /* contains the linked list in wich the current string is stored */
    curr_str_words_list words_list;
} type_tokenizer;

bool words_list_is_empty(const type_tokenizer *tknzer);
/*
    Checks if the `tknzer->words_list` contains no `word_item`'s.
RECEIVES:
    - `tknzer` address of `type_tokenizer` structure;
RETURNES:
    - `true` if `tknzer->words_list` is empty, `false` otherwise */

void init_tokenizer(type_tokenizer *tknzer);
/*
    Initializes `type_tokenizer` structure.
RECEIVES:
    - `tknzer` address of `type_tokenizer` structure */

void reset_tokenizer_variables(type_tokenizer *tknzer);
/*
    Resets the `type_tokenizer` structure fields to get ready to work with the next
`my_shell` interpreter command.
RECEIVES:
    - `tknzer` address of `type_tokenizer` structure */

void free_list_of_words(curr_str_words_list *link_list);
/*
    Frees memory allocated for `tknzer->words_list`;
RECEIVES:
    - `link_list` address of words list (`tknzer->words_list`) */

void free_tokenizer(type_tokenizer *tknzer);
/*
    Frees memory, dynamically allocated for the `type_tokenizer` structure;
RECEIVES:
    - `tknzer` address of `type_tokenizer` structure */

#endif
