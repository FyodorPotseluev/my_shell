/* constants.h */

#ifndef CONSTANTS_H_INCLUDED
#define CONSTANTS_H_INCLUDED

#include <stdbool.h>

extern char *environ;

enum consts {
    init_tmp_wrd_arr_len    = 16,
    init_cmd_line_arr_len   = 8
};

#define ERR_SMTH_STRANGE_HAPPEND    "%s, %d: Something went wrong\n"
#define ERR_NO_SUCH_FILE            "my_shell: %s: No such file or directory\n"
#define ERR_BACKGROUND_OPERATOR_IN_THE_END_OF_STR \
    "my_shell: Error: & not in the end of string\n"
#define ERR_SEPARATOR_AFTER_IO_REDIRECTION \
    "my_shell: Error: separator right after IO redirection\n"
#define ERR_2ND_FILE_NAME_AFTER_IO_REDIRECTION \
    "my_shell: Error: 2nd file name after IO redirection\n"
#define ERR_IO_REDIRECTION_USED_TWICE \
    "my_shell: Error: > or < used twice, or > together with >>\n"
#define ERR_PIPE_OPERATOR_MISUSE \
    "my_shell: Error: | at start of the string or two | in a row\n"

typedef enum tag_error_code {
    no_error,
    incorrect_char_escaping,
    background_operator_not_in_the_end_of_str,
    separator_right_after_input_or_output_redirection,
    second_simple_word_right_after_input_or_output_redirecton,
    input_or_output_separator_used_in_line_twice,
    pipe_operator_at_start_of_str,
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

typedef struct tag_io_status {
    bool redirection;
    bool waiting_for_file;
    const char *redirection_file;
} io_status;

typedef struct tag_execvp_cmd_line {
    char **arr;
    int arr_len;
    int pid;
    io_status input;
    io_status output_overwrite;
    io_status output_append;
    struct tag_execvp_cmd_line *next;
} execvp_cmd_line;

typedef struct tag_cmd_lines_list {
    execvp_cmd_line *first;
    execvp_cmd_line *last;
    int list_len;
    bool background_execution;
} cmd_lines_list;

typedef struct tag_pipeline_item {
    int fd[2];
    struct tag_pipeline_item *next;
} pipeline_item;

typedef struct tag_pipeline_list {
    pipeline_item *first;
} pipeline_list;

typedef struct tag_string {
    bool word_ended, str_ended, quotation, char_escaping;
    int c;
    error_code err_code;
    /* contains the array in which the current word is formed */
    curr_word_dynamic_char_arr tmp_wrd;
    /* contains the linked list in wich the current string is stored */
    curr_str_words_list words_list;
    /* contains the array designated for the `execvp` call */
    cmd_lines_list cmd_line;
    /* contains the pipes linking processes into a pipeline */
    pipeline_list pipeline;
} string;

#endif
