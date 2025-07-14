/* main.c */

#if !defined(EXEC_MODE) && !defined(PRINT_TOKENS_MODE) && !defined(LOG)
#error Please define either EXEC_MODE or PRINT_TOKENS_MODE or LOG
#endif

#include "cmd_line.h"
#include "err_code.h"
#if defined(LOG)
#  include "handle_err.h"
#  include <fcntl.h>
#  include <sys/stat.h>
#  include <sys/types.h>
#endif
#if defined(EXEC_MODE) || defined(LOG)
#  include "make_cmd_line.h"
#  include "execute_command.h"
#elif defined(PRINT_TOKENS_MODE)
#  include "print_tokens.h"
#endif
#include "handle_signals.h"
#include "input.h"
#include "parse_input_string.h"
#include "read_interactive_string.h"
#include "reprogram_terminal.h"
#include "ternary_search_tree.h"
#include "tokenizer.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#if defined(LOG)
int log_fd;
#endif

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

static void handle_error(
    error_code error, type_tokenizer *tknzer, type_cmd_line *cmdline
)
{
    print_error(error);
    free_list_of_words(&tknzer->words_list);
    if (cmdline)
        free_cmd_line(cmdline);
    reset_tokenizer_variables(tknzer);
}

static void process_parsed_string(type_tokenizer *tknzer)
{
#if defined(EXEC_MODE) || defined(LOG)
    type_cmd_line cmdline;
#endif
    /* provide immutable reference of `tknzer` to `parse_input_str` module */
    error_code error = there_is_a_parsing_error(tknzer);
    if (error) {
        handle_error(error, tknzer, NULL);
        return;
    }
#if defined(PRINT_TOKENS_MODE)
    print_tokens(&tknzer->words_list);
    free_list_of_words(&tknzer->words_list);
#elif defined(EXEC_MODE) || defined(LOG)
    init_cmd_line(&cmdline);
    /* provide mutable reference of `cmdline` to `make_cmd_line` module */
    /* provide mutable reference of `tknzer` to `make_cmd_line` module */
    error = make_cmd_line(&cmdline, tknzer);
    if (error) {
        handle_error(error, tknzer, &cmdline);
        return;
    }
    free_list_of_words(&tknzer->words_list);
    /* provide mutable reference of `cmdline` to `execute_command` module */
    execute_command(&cmdline);
    free_cmd_line(&cmdline);
#endif
    reset_tokenizer_variables(tknzer);
}

int main()
{
    type_input input;
    type_tokenizer tknzer;
    type_tst *path_tree = NULL;
    init_input(&input);
    init_tokenizer(&tknzer);
    init_path_tree(&path_tree);
#if defined(LOG)
    log_fd = open("log", O_WRONLY|O_CREAT|O_TRUNC|O_APPEND, 0666);
    error_handling(log_fd, __FILE__, __LINE__, "open");
#endif
    set_signal_disposition(SIGCHLD, handle_background_zombie_process);
    reprogram_terminal_attributes();
    while (true) {
        /* we temporarily pass the ownership of `input`, `tknzr` and `path_tree`
        (the program shutdown is handled there) */
        read_interactive_string(&input, &tknzer, path_tree);
        for (input.cur_idx = 0; ; input.cur_idx++) {
            /* provide immutable references of `tknzer` and `input` to
            `parse_input_str` module */
            parse_next_character(&tknzer, &input);
            if (tknzer.str_ended) {
                process_parsed_string(&tknzer);
                break;
            }
        }
    }
}
