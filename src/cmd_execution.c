/* cmd_execution.c */

#if !defined(EXEC_MODE) && !defined(PRINT_TOKENS_MODE)
#error Please define either EXEC_MODE or PRINT_TOKENS_MODE
#endif

#include "cmd_execution.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

bool words_list_is_empty(const string *str)
{
    return (!str->words_list.first) ? true : false;
}

static bool there_are_possible_zombies_left(int res)
{
    /* if we get a valid pid */
    return ((res != 0) && (res != -1)) ? true : false;
}

void cleanup_background_zombies()
{
    int res;
    do {
        res = wait4(-1, NULL, WNOHANG, NULL);
    } while (there_are_possible_zombies_left(res));
}

#if defined(PRINT_TOKENS_MODE)
static void print_separator_value(separator_type separator_val)
{
    switch (separator_val) {
        case (none):
            fprintf(stderr, "my_shell: Error: something went wrong :/\n");
            break;
        case (background_operator):
            printf("[background_operator]\n");
            break;
        case (and_operator):
            printf("[and_operator]\n");
            break;
        case (output_redirection):
            printf("[output_redirection]\n");
            break;
        case (output_append_redirection):
            printf("[output_append_redirection]\n");
            break;
        case (pipe_operator):
            printf("[pipe_operator]\n");
            break;
        case (or_operator):
            printf("[or_operator]\n");
            break;
        case (input_redirection):
            printf("[input_redirection]\n");
            break;
        case (command_separator):
            printf("[command_separator]\n");
            break;
        case (open_parenthesis):
            printf("[open_parenthesis]\n");
            break;
        case (close_parenthesis):
            printf("[close_parenthesis]\n");
    }
}

static void print_list_of_words(const curr_str_words_list *words_list)
{
    word_item *p = words_list->first;
    while (p) {
        if (p->separator_val != none)
            print_separator_value(p->separator_val);
        else
            printf("[%s]\n", p->word ? p->word : "");
        p = p->next;
    }
}
#elif defined(EXEC_MODE)
static void error_handling(
    int res, const char *file_name, int line_num, const char *cmd
)
{
    if (res == -1) {
        fprintf(stderr, "%s, %d, %s: ", file_name, line_num, cmd);
        perror("");
        fflush(stderr);
    }
}

static void increase_cmd_line_array_length(string *str)
{
    free(str->cmd_line.arr);
    while (str->cmd_line.arr_len < str->words_list.len)
        str->cmd_line.arr_len *= 2;
    str->cmd_line.arr = malloc(str->cmd_line.arr_len * sizeof(char*));
}

static error_code handle_possible_separator(
    string *str, const word_item *curr_word, int idx,
    bool *background_execution, bool *next_step
)
{
    if (curr_word->separator_val == background_operator) {
        str->cmd_line.arr[idx] = NULL;
        *background_execution = true;
        *next_step = true;
    } else
    if (*background_execution)
        return background_operator_not_in_the_end_of_str;
    else
    if (curr_word->separator_val != none)
        return not_implemented_feature;
    return no_error;
}

static error_code transform_words_list_into_cmd_line_arr(
    string *str, bool *background_execution
)
{
    word_item *p = str->words_list.first;
    int i = 0;
    if (str->cmd_line.arr_len < str->words_list.len)
        increase_cmd_line_array_length(str);
    for (;; p=p->next, i++) {
        if (!p) {
            str->cmd_line.arr[i] = NULL;
            break;
        } else {
            bool next_step = false;
            error_code err;
            err = handle_possible_separator(
                str, p, i, background_execution, &next_step
            );
            if (err)
                return err;
            if (next_step)
                continue;
            str->cmd_line.arr[i] = p->word;
        }
    }
    return 0;
}

static void change_to_home_directory()
{
    int res;
    const char *home_dir = getenv("HOME");
    if (!home_dir) {
        fprintf(stderr, "%s, %d, %s: ", __FILE__, __LINE__, "getenv");
        perror("");
        return;
    }
    res = chdir(home_dir);
    error_handling(res, __FILE__, __LINE__, "chdir");
}

static void handle_change_dir_command(const string *str)
{
    if (!str->cmd_line.arr[1])
        change_to_home_directory();
    else
    if (!str->cmd_line.arr[2]) {
        int res = chdir(str->cmd_line.arr[1]);
        error_handling(res, __FILE__, __LINE__, "chdir");
    } else
        fprintf(stderr, "my_shell: cd: too many arguments\n");
}

static bool change_dir_command(const string *str)
{
    return (0 == strcmp("cd", str->cmd_line.arr[0])) ? true : false;
}

static void print_error(error_code err)
{
    switch (err) {
        case (no_error):
            fprintf(stderr, "%s, %d: Something went wrong\n", __FILE__, __LINE__);
            break;
        case (incorrect_char_escaping):
            fprintf(stderr, "%s, %d: Something went wrong\n", __FILE__, __LINE__);
            break;
        case (background_operator_not_in_the_end_of_str):
            fprintf(
                stderr,
                "my_shell: Error: background operator not in the end of string\n"
            );
            break;
        case (not_implemented_feature):
            fprintf(stderr, "my_shell: feature not implemented yet\n");
    }
}

static void handle_zombies(bool background_execution, int pid)
{
    int res;
    if (background_execution)
        cleanup_background_zombies();
    else {
        /* cleanup the single foreground zombie */
        do {
            res = wait(NULL);
            error_handling(res, __FILE__, __LINE__, "wait");
        } while (res != pid);
    }
}

static int launch_process(const string *str)
{
    int pid;
    fflush(stderr);
    pid = fork();
    error_handling(pid, __FILE__, __LINE__, "fork");
    if (pid == 0) {
        execvp(str->cmd_line.arr[0], str->cmd_line.arr);
        fprintf(stderr, "%s, %d, %s: ", __FILE__, __LINE__, "execvp");
        perror("");
        fflush(stderr);
        _exit(1);
    }
    return pid;
}
#endif

void execute_command(string *str)
{
#if defined(PRINT_TOKENS_MODE)
    print_list_of_words(&str->words_list);
#elif defined(EXEC_MODE)
    bool background_execution = false;
    int pid;
    error_code err = 0;
    if (words_list_is_empty(str))
        return;
    err = transform_words_list_into_cmd_line_arr(str, &background_execution);
    if (err) {
        print_error(err);
        return;
    }
    if (change_dir_command(str)) {
        handle_change_dir_command(str);
        return;
    }
    pid = launch_process(str);
    handle_zombies(background_execution, pid);
#endif
}
