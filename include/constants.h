/* constants.h */

#ifndef CONSTANTS_H_INCLUDED
#define CONSTANTS_H_INCLUDED

#include <stdbool.h>

extern char *environ;

enum consts {
    init_tmp_wrd_arr_len    = 16,
    init_cmd_line_arr_len   = 16
};

typedef enum tag_error_code {
    no_error,
    incorrect_char_escaping,
    background_operator_not_in_the_end_of_str,
    not_implemented_feature
} error_code;

typedef enum tag_separator_type {
    none,
    background_operator,
    and_operator,
    output_redirection,
    output_append_redirection,
    pipe_operator,
    or_operator,
    input_redirection,
    command_separator,
    open_parenthesis,
    close_parenthesis
} separator_type;

typedef struct tag_word_item {
    char *word;
    separator_type separator_val;
    struct tag_word_item *next;
} word_item;

typedef struct tag_curr_str_words_list {
    word_item *first;
    word_item *last;
    int len;
} curr_str_words_list;

typedef struct tag_curr_word_dynamic_char_arr {
    char *arr;
    int idx;
    int arr_len;
} curr_word_dynamic_char_arr;

typedef struct tag_execvp_cmd_line {
    char **arr;
    int arr_len;
} execvp_cmd_line;

typedef struct tag_string {
    bool word_ended, str_ended, quotation, char_escaping;
    int c;
    error_code err_code;
    /* contains the array in which the current word is formed */
    curr_word_dynamic_char_arr tmp_wrd;
    /* contains the linked list in wich the current string is stored */
    curr_str_words_list words_list;
    /* contains the array designated for the `execvp` call */
    execvp_cmd_line cmd_line;
} string;

#endif
