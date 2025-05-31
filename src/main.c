/* main.c */

#if !defined(EXEC_MODE) && !defined(PRINT_TOKENS_MODE)
#error Please define either EXEC_MODE or PRINT_TOKENS_MODE
#endif

#include "cmd_line.h"
#include "err_code.h"
#if defined(EXEC_MODE)
#  include "make_cmd_line.h"
#  include "execute_command.h"
#elif defined(PRINT_TOKENS_MODE)
#  include "print_tokens.h"
#endif
#include "handle_signals.h"
#include "parse_string.h"
#include "str.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define ERR_SMTH_STRANGE_HAPPEND \
    "%s, %d: Something went wrong\n"
#define ESC_ERR \
    "my_shell: Error: only the characters `\"` and `\\` can be escaped\n"
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

static void print_error(error_code err)
{
    switch (err) {
        case (no_error):
            /* we weren't supposed to get to this place of the program */
            fprintf(stderr, ERR_SMTH_STRANGE_HAPPEND, __FILE__, __LINE__);
            break;
        case (incorrect_char_escaping):
            fprintf(stderr, ESC_ERR);
            break;
        case (unmatched_quotes):
            fprintf(stderr, "my_shell: Error: unmatched quotes\n");
            break;
        case (background_operator_not_in_the_end_of_str):
            fprintf(stderr, ERR_BACKGROUND_OPERATOR_IN_THE_END_OF_STR);
            break;
        case (separator_right_after_input_or_output_redirection):
            fprintf(stderr, ERR_SEPARATOR_AFTER_IO_REDIRECTION);
            break;
        case (second_simple_word_right_after_input_or_output_redirecton):
            fprintf(stderr, ERR_2ND_FILE_NAME_AFTER_IO_REDIRECTION);
            break;
        case (input_or_output_separator_used_in_line_twice):
            fprintf(stderr, ERR_IO_REDIRECTION_USED_TWICE);
            break;
        case (pipe_operator_at_start_of_str):
            fprintf(stderr, ERR_PIPE_OPERATOR_MISUSE);
            break;
        case (not_implemented_feature):
            fprintf(stderr, "my_shell: feature not implemented yet\n");
    }
}

static void handle_error(error_code error, string *str, cmd_line *cmdline)
{
    print_error(error);
    free_list_of_words(&str->words_list);
    if (cmdline)
        free_cmd_line(cmdline);
    reset_str_variables(str);
}

static void process_end_of_string(string *str)
{
#if defined(EXEC_MODE)
    cmd_line cmdline;
#endif
    /* provide immutable reference of `str` to `parse_string` module */
    error_code error = there_is_a_parsing_error(str);
    if (error) {
        handle_error(error, str, NULL);
        return;
    }
#if defined(PRINT_TOKENS_MODE)
    print_tokens(&str->words_list);
    free_list_of_words(&str->words_list, &free_list_of_words_strings);
#elif defined(EXEC_MODE)
    init_cmd_line(&cmdline);
    /* provide mutable reference of `cmdline` to `make_cmd_line` module */
    /* provide mutable reference of `str` to `make_cmd_line` module */
    error = make_cmd_line(&cmdline, str);
    if (error) {
        handle_error(error, str, &cmdline);
        return;
    }
    free_list_of_words(&str->words_list);
    /* provide mutable reference of `cmdline` to `execute_command` module */
    execute_command(&cmdline);
    free_cmd_line(&cmdline);
#endif
    reset_str_variables(str);
    printf("> ");
}

int main()
{
    string str;
    init_str(&str);
    set_signal_disposition(SIGCHLD, handle_background_zombie_process);
    printf("> ");
    while ((str.c=getchar_signal_protected()) != EOF) {
        /* provide mutable reference of `str` to `parse_string` module */
        process_character(&str);
        if (str.str_ended)
            process_end_of_string(&str);
    }
    free_str(&str);
    printf("^D\n");
    return 0;
}
