/* cmd_execution.c */

#if !defined(EXEC_MODE) && !defined(PRINT_TOKENS_MODE)
#error Please define either EXEC_MODE or PRINT_TOKENS_MODE
#endif

#include "cmd_execution.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct tag_io_status {
    bool redirection;
    bool waiting_for_file;
    const char *redirection_file;
} io_status;

typedef struct tag_command_line_status {
    bool background_execution;
    io_status input;
    io_status output_overwrite;
    io_status output_append;
} command_line_status;

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

static bool separator_right_after_io_redirecton(
    const word_item *curr_word, command_line_status *cmdline_status
)
{
    bool the_word_is_separator = (curr_word->separator_val != none);
    bool io_redirection_waiting_for_file =
        (cmdline_status->input.waiting_for_file ||
        cmdline_status->output_overwrite.waiting_for_file ||
        cmdline_status->output_append.waiting_for_file);

    if (the_word_is_separator && io_redirection_waiting_for_file)
        return true;
    else
        return false;
}

static bool second_simple_word_right_after_io_redirecton(
    const word_item *curr_word, command_line_status *cmdline_status
)
{
    bool simple_word = (curr_word->separator_val == none);
    bool at_least_one_io_redirection =
        (cmdline_status->input.redirection ||
        cmdline_status->output_overwrite.redirection ||
        cmdline_status->output_append.redirection);
    bool io_redirection_not_waiting_for_file =
        (!cmdline_status->input.waiting_for_file &&
        !cmdline_status->output_overwrite.waiting_for_file &&
        !cmdline_status->output_append.waiting_for_file);

    if (simple_word &&
        at_least_one_io_redirection &&
        io_redirection_not_waiting_for_file)
    {
        return true;
    } else {
        return false;
    }
}

static void appoint_stream_redirection(
    io_status *stream, string *str,
    const word_item *curr_word, int idx, bool *next_step
)
{
    stream->redirection_file = curr_word->word;
    stream->waiting_for_file = false;
    str->cmd_line.arr[idx] = NULL;
    *next_step = true;
}

enum tag_status { error = -1, move_on, completed };

static enum tag_status appoint_io_redirection_file(
    string *str, const word_item *curr_word, int idx,
    command_line_status *cmdline_status, bool *next_step
)
{
    if (curr_word->separator_val == none) {
        if (cmdline_status->input.waiting_for_file) {
            appoint_stream_redirection(
                &cmdline_status->input, str, curr_word, idx, next_step
            );
            return completed;
        } else
        if (cmdline_status->output_overwrite.waiting_for_file) {
            appoint_stream_redirection(
                &cmdline_status->output_overwrite, str, curr_word, idx,next_step
            );
            return completed;
        } else
        if (cmdline_status->output_append.waiting_for_file) {
            appoint_stream_redirection(
                &cmdline_status->output_append, str, curr_word, idx, next_step
            );
            return completed;
        } else
            return move_on;
    } else
        return move_on;
}

static enum tag_status start_background_execution(
    string *str, const word_item *curr_word, int idx,
    command_line_status *cmdline_status, bool *next_step
)
{
    if (curr_word->separator_val == background_operator) {
        str->cmd_line.arr[idx] = NULL;
        cmdline_status->background_execution = true;
        *next_step = true;
        return completed;
    } else
        return move_on;
}

static enum tag_status toggle_stream_redirection(
    io_status *stream, bool error_condition,
    string *str, int idx, bool *next_step
)
{
    if (error_condition)
        return error;
    stream->redirection = true;
    stream->waiting_for_file = true;
    str->cmd_line.arr[idx] = NULL;
    *next_step = true;
    return completed;
}

static enum tag_status toggle_io_redirection(
    string *str, const word_item *curr_word, int idx,
    command_line_status *cmdline_status, bool *next_step
)
{
    if (curr_word->separator_val == input_redirection) {
        bool error_condition = (cmdline_status->input.redirection);
        return toggle_stream_redirection(
            &cmdline_status->input, error_condition, str, idx, next_step
        );
    } else
    if (curr_word->separator_val == output_redirection) {
        bool error_condition =
            (cmdline_status->output_overwrite.redirection ||
            cmdline_status->output_append.redirection);
        return toggle_stream_redirection(
            &cmdline_status->output_overwrite,
            error_condition, str, idx, next_step
        );
    } else
    if (curr_word->separator_val == output_append_redirection) {
        bool error_condition =
            (cmdline_status->output_overwrite.redirection ||
            cmdline_status->output_append.redirection);
        return toggle_stream_redirection(
            &cmdline_status->output_append,
            error_condition, str, idx, next_step
        );
    } else
        return move_on;
}

static error_code handle_possible_separator(
    string *str, const word_item *curr_word, int idx,
    command_line_status *cmdline_status, bool *next_step
)
{
    enum tag_status { error = -1, move_on, completed } status;
    if (cmdline_status->background_execution)
        return background_operator_not_in_the_end_of_str;
    if (separator_right_after_io_redirecton(curr_word, cmdline_status))
        return separator_right_after_input_or_output_redirection;
    if (second_simple_word_right_after_io_redirecton(curr_word, cmdline_status))
        return second_simple_word_right_after_input_or_output_redirecton;
    status =
        appoint_io_redirection_file(
            str, curr_word, idx, cmdline_status, next_step
        );
    if (status == completed)
        return no_error;
    status =
        start_background_execution(
            str, curr_word, idx, cmdline_status, next_step
        );
    if (status == completed)
        return no_error;
    status = toggle_io_redirection(
        str, curr_word, idx, cmdline_status, next_step
    );
    if (status == completed)
        return no_error;
    if (status == error)
        return input_or_output_separator_used_in_line_twice;
    if (curr_word->separator_val != none)
        return not_implemented_feature;
    return no_error;
}

static error_code transform_words_list_into_cmd_line_arr(
    string *str, command_line_status *cmdline_status
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
                str, p, i, cmdline_status, &next_step
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
    if (!str->cmd_line.arr[0])
        return false;
    if (0 == strcmp("cd", str->cmd_line.arr[0]))
        return true;
    else
        return false;
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
                "my_shell: Error: & not in the end of string\n"
            );
            break;
        case (separator_right_after_input_or_output_redirection):
            fprintf(
                stderr,
                "my_shell: Error: separator right after IO redirection\n"
            );
            break;
        case (second_simple_word_right_after_input_or_output_redirecton):
            fprintf(
                stderr,
                "my_shell: Error: 2nd file name after IO redirection\n"
            );
            break;
        case (input_or_output_separator_used_in_line_twice):
            fprintf(
                stderr,
                "my_shell: Error: > or < used twice, or > together with >>\n"
            );
            break;
        case (not_implemented_feature):
            fprintf(stderr, "my_shell: feature not implemented yet\n");
    }
}

static void handle_zombies(command_line_status *cmdline_status, int pid)
{
    int res;
    if ((cmdline_status->background_execution) || (pid == -1))
        cleanup_background_zombies();
    else {
        /* cleanup the single foreground zombie */
        do {
            res = wait(NULL);
            error_handling(res, __FILE__, __LINE__, "wait");
        } while (res != pid);
    }
}

static const char *get_output_file(const command_line_status *cmdline_status)
{
    if (cmdline_status->output_overwrite.redirection_file)
        return cmdline_status->output_overwrite.redirection_file;
    else
    if (cmdline_status->output_append.redirection_file)
        return cmdline_status->output_append.redirection_file;
    else
        return NULL;
}

static int open_io_redirecton_files(
    int *fd_input, int *fd_output, const char *input_file,
    const char *output_file, const command_line_status *cmdline_status
)
{
    if (cmdline_status->input.redirection) {
        *fd_input = open(input_file, O_RDONLY);
        if ((*fd_input == -1) && (errno == ENOENT)) {
            fprintf(
                stderr, "my_shell: %s: No such file or directory", input_file
            );
            return -1;
        }
    }
    if (cmdline_status->output_overwrite.redirection)
        *fd_output = open(output_file, O_WRONLY|O_CREAT|O_TRUNC, 0666);
    if (cmdline_status->output_append.redirection)
        *fd_output = open(output_file, O_WRONLY|O_CREAT|O_APPEND, 0666);
    error_handling(*fd_input, __FILE__, __LINE__, "open");
    error_handling(*fd_output, __FILE__, __LINE__, "open");
    return 0;
}

static void close_io_redirection_files(int fd_input, int fd_output)
{
    int res;
    if (fd_input) {
        res = close(fd_input);
        error_handling(res, __FILE__, __LINE__, "close");
    }
    if (fd_output) {
        res = close(fd_output);
        error_handling(res, __FILE__, __LINE__, "close");
    }
}

static void io_redirection(
    int fd_input, int fd_output, const command_line_status *cmdline_status
)
{
    int res;
    if (cmdline_status->input.redirection) {
        res = dup2(fd_input, 0);
        error_handling(res, __FILE__, __LINE__, "dup2");
    }
    if (cmdline_status->output_overwrite.redirection ||
        cmdline_status->output_append.redirection)
    {
        res = dup2(fd_output, 1);
        error_handling(res, __FILE__, __LINE__, "dup2");
    }
    close_io_redirection_files(fd_input, fd_output);
}

static int launch_process(
    const string *str, command_line_status *cmdline_status
)
{
    int fd_input = 0, fd_output = 0, pid, res;
    const char *input_file = cmdline_status->input.redirection_file;
    const char *output_file = get_output_file(cmdline_status);
    res = open_io_redirecton_files(
        &fd_input, &fd_output, input_file, output_file, cmdline_status
    );
    if (res == -1)
        return -1;
    fflush(stderr);
    pid = fork();
    error_handling(pid, __FILE__, __LINE__, "fork");
    if (pid == 0) {
        io_redirection(fd_input, fd_output, cmdline_status);
        if (str->cmd_line.arr[0]) {
            execvp(str->cmd_line.arr[0], str->cmd_line.arr);
            fprintf(
                stderr, "%s, %d, %s: %s:",
                __FILE__, __LINE__, "execvp", str->cmd_line.arr[0]
            );
            perror("");
        }
        fflush(stderr);
        _exit(1);
    }
    close_io_redirection_files(fd_input, fd_output);
    return pid;
}
#endif

void execute_command(string *str)
{
#if defined(PRINT_TOKENS_MODE)
    print_list_of_words(&str->words_list);
#elif defined(EXEC_MODE)
    int pid;
    command_line_status cmdline_status = {
        false,
        { false, false, NULL },
        { false, false, NULL },
        { false, false, NULL }
    };
    error_code err = 0;
    if (words_list_is_empty(str))
        return;
    err = transform_words_list_into_cmd_line_arr(str, &cmdline_status);
    if (err) {
        print_error(err);
        return;
    }
    if (change_dir_command(str)) {
        handle_change_dir_command(str);
        return;
    }
    pid = launch_process(str, &cmdline_status);
    handle_zombies(&cmdline_status, pid);
#endif
}
