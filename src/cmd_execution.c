/* cmd_execution.c */

#if !defined(EXEC_MODE) && !defined(PRINT_TOKENS_MODE)
#error Please define either EXEC_MODE or PRINT_TOKENS_MODE
#endif

#include "cmd_execution.h"
#include "error_handling.h"
#include "zombie_handling.h"
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

static void reset_io_status(io_status *io_stat)
{
    io_stat->redirection = false;
    io_stat->waiting_for_file = false;
    io_stat->redirection_file = NULL;
}

void reset_cmd_line_item(execvp_cmd_line *item)
{
    item->pid = 0;
    reset_io_status(&item->input);
    reset_io_status(&item->output_overwrite);
    reset_io_status(&item->output_append);
    /* item->next = NULL; */
}

void init_cmd_line_item(execvp_cmd_line *item)
{
    item->arr_len = init_cmd_line_arr_len;
    item->arr = malloc(item->arr_len * sizeof(char*));
    reset_cmd_line_item(item);
    item->next = NULL;
}

bool words_list_is_empty(const string *str)
{
    return (!str->words_list.first);
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
static void increase_cmd_line_array_length(execvp_cmd_line *cmdline)
{
    cmdline->arr_len *= 2;
    cmdline->arr = realloc(cmdline->arr, cmdline->arr_len * sizeof(char*));
}

static bool pipe_operator_at_start_of_cmdline(
    const word_item *curr_word, int cmdline_idx
)
{
    return(
        curr_word->separator_val == pipe_operator &&
        cmdline_idx == 0
    );
}

static bool separator_right_after_io_redirecton(
    const string *str, const word_item *curr_word
)
{
    const execvp_cmd_line *cmdline = str->cmd_line.last;

    bool the_word_is_separator = (curr_word->separator_val != none);

    bool io_redirection_waiting_for_file =
        (cmdline->input.waiting_for_file ||
        cmdline->output_overwrite.waiting_for_file ||
        cmdline->output_append.waiting_for_file);

    return (the_word_is_separator && io_redirection_waiting_for_file);
}

static bool second_simple_word_right_after_io_redirecton(
    string *str, const word_item *curr_word 
)
{
    const execvp_cmd_line *cmdline = str->cmd_line.last;

    bool simple_word = (curr_word->separator_val == none);

    bool at_least_one_io_redirection =
        (cmdline->input.redirection ||
        cmdline->output_overwrite.redirection ||
        cmdline->output_append.redirection);

    bool io_redirection_not_waiting_for_file =
        (!cmdline->input.waiting_for_file &&
        !cmdline->output_overwrite.waiting_for_file &&
        !cmdline->output_append.waiting_for_file);

    return(
        simple_word &&
        at_least_one_io_redirection &&
        io_redirection_not_waiting_for_file
    );
}

static void appoint_stream_redirection(
    io_status *stream, execvp_cmd_line *cmdline,
    const word_item *curr_word, int idx, bool *next_step
)
{
    stream->redirection_file = curr_word->word;
    stream->waiting_for_file = false;
    cmdline->arr[idx] = NULL;
    *next_step = true;
}

enum tag_status { error = -1, move_on, completed };

static enum tag_status appoint_io_redirection_file(
    string *str, const word_item *curr_word, int idx, bool *next_step
)
{
    execvp_cmd_line *cmdline = str->cmd_line.last;
    if (curr_word->separator_val == none) {
        if (cmdline->input.waiting_for_file) {
            appoint_stream_redirection(
                &cmdline->input, cmdline, curr_word, idx, next_step
            );
            return completed;
        } else
        if (cmdline->output_overwrite.waiting_for_file) {
            appoint_stream_redirection(
                &cmdline->output_overwrite, cmdline, curr_word, idx, next_step
            );
            return completed;
        } else
        if (cmdline->output_append.waiting_for_file) {
            appoint_stream_redirection(
                &cmdline->output_append, cmdline, curr_word, idx, next_step
            );
            return completed;
        } else
            return move_on;
    } else
        return move_on;
}

static enum tag_status start_background_execution(
    string *str, const word_item *curr_word, int idx, bool *next_step
)
{
    if (curr_word->separator_val == background_operator) {
        str->cmd_line.last->arr[idx] = NULL;
        str->cmd_line.background_execution = true;
        *next_step = true;
        return completed;
    } else
        return move_on;
}

void add_new_cmd_line_item(cmd_lines_list *cmdline)
{
    /* add item at the end of `cmd_line` link list */
    cmdline->last->next = malloc(sizeof(execvp_cmd_line));
    cmdline->last = cmdline->last->next;
    init_cmd_line_item(cmdline->last);
    cmdline->list_len++;
}

void add_new_pipeline_item(pipeline_list *pipeline)
{
    /* add item at the beginning of `pipeline` link list */
    int res;
    pipeline_item *tmp = malloc(sizeof(pipeline_item));
    res = pipe(tmp->fd);
    error_handling(res, __FILE__, __LINE__, "pipe");
    tmp->next = pipeline->first;
    pipeline->first = tmp;
}

static enum tag_status split_cmdline_and_add_pipeline(
    string *str, const word_item *curr_word, int *idx, bool *next_step
)
{
    if (curr_word->separator_val == pipe_operator) {
        str->cmd_line.last->arr[*idx] = NULL;
        add_new_cmd_line_item(&str->cmd_line);
        add_new_pipeline_item(&str->pipeline);
        (*idx) = -1;        /* on the next iteration it will be 0 */
        *next_step = true;
        return completed;
    } else
        return move_on;
}

static enum tag_status toggle_stream_redirection(
    io_status *stream, execvp_cmd_line *cmdline,
    bool error_condition, int idx, bool *next_step
)
{
    if (error_condition)
        return error;
    stream->redirection = true;
    stream->waiting_for_file = true;
    cmdline->arr[idx] = NULL;
    *next_step = true;
    return completed;
}

static enum tag_status toggle_io_redirection(
    string *str, const word_item *curr_word, int idx, bool *next_step
)
{
    execvp_cmd_line *cmdline = str->cmd_line.last;
    if (curr_word->separator_val == input_redirection) {
        bool error_condition = (cmdline->input.redirection);
        return toggle_stream_redirection(
            &cmdline->input, cmdline, error_condition, idx, next_step
        );
    } else
    if (curr_word->separator_val == output_redirection) {
        bool error_condition = (
            cmdline->output_overwrite.redirection ||
            cmdline->output_append.redirection
        );
        return toggle_stream_redirection(
            &cmdline->output_overwrite, cmdline, error_condition, idx, next_step
        );
    } else
    if (curr_word->separator_val == output_append_redirection) {
        bool error_condition = (
            cmdline->output_overwrite.redirection ||
            cmdline->output_append.redirection
        );
        return toggle_stream_redirection(
            &cmdline->output_append, cmdline, error_condition, idx, next_step
        );
    } else
        return move_on;
}

static error_code handle_possible_separator(
    string *str, const word_item *curr_word, int *idx, bool *next_step
)
{
    enum tag_status { error = -1, move_on, completed } status;
    if (str->cmd_line.background_execution)
        return background_operator_not_in_the_end_of_str;
    if (pipe_operator_at_start_of_cmdline(curr_word, *idx))
        return pipe_operator_at_start_of_str;
    if (separator_right_after_io_redirecton(str, curr_word))
        return separator_right_after_input_or_output_redirection;
    if (second_simple_word_right_after_io_redirecton(str, curr_word))
        return second_simple_word_right_after_input_or_output_redirecton;
    status = split_cmdline_and_add_pipeline(str, curr_word, idx, next_step);
    if (status == completed)
        return no_error;
    status = appoint_io_redirection_file(str, curr_word, *idx, next_step);
    if (status == completed)
        return no_error;
    status = start_background_execution(str, curr_word, *idx, next_step);
    if (status == completed)
        return no_error;
    status = toggle_io_redirection(str, curr_word, *idx, next_step);
    if (status == completed)
        return no_error;
    if (status == error)
        return input_or_output_separator_used_in_line_twice;
    if (curr_word->separator_val != none)
        return not_implemented_feature;
    return no_error;
}

static error_code transform_words_list_into_cmd_line_arr(string *str)
{
    word_item *p = str->words_list.first;
    int i = 0;
    for (;; p=p->next, i++) {
        if (str->cmd_line.last->arr_len == i)
            increase_cmd_line_array_length(str->cmd_line.last);
        if (!p) {
            str->cmd_line.last->arr[i] = NULL;
            break;
        } else {
            bool next_step = false;
            error_code err;
            err = handle_possible_separator(str, p, &i, &next_step);
            if (err)
                return err;
            if (next_step)
                continue;
            str->cmd_line.last->arr[i] = p->word;
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

static void handle_change_dir_command(const execvp_cmd_line *cmdline)
{
    if (!cmdline->arr[1])
        change_to_home_directory();
    else
    if (!cmdline->arr[2]) {
        int res = chdir(cmdline->arr[1]);
        error_handling(res, __FILE__, __LINE__, "chdir");
    } else
        fprintf(stderr, "my_shell: cd: too many arguments\n");
}

static bool change_dir_command(const execvp_cmd_line *cmdline)
{
    if (!cmdline->arr[0])
        return false;
    if (0 == strcmp("cd", cmdline->arr[0]))
        return true;
    else
        return false;
}

static void print_error(error_code err)
{
    switch (err) {
        case (no_error):
            /* we weren't supposed to get to this place of the program */
            fprintf(stderr, ERR_SMTH_STRANGE_HAPPEND, __FILE__, __LINE__);
            break;
        case (incorrect_char_escaping):
            /* this error had to be handled in main -> process_end_of_string ->
            -> report_if_error */
            fprintf(stderr, ERR_SMTH_STRANGE_HAPPEND, __FILE__, __LINE__);
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

static void wait_for_cmd_linde_item(const execvp_cmd_line *item)
{
    int res;
    if (item->pid != 0) {
        res = waitpid(item->pid, NULL, WNOHANG);
        if (res == 0)
            /* the process exists */
            do {
                res = wait(NULL);
                error_handling(res, __FILE__, __LINE__, "wait");
            } while (res != item->pid);
        else
        if ((res == -1) && (errno == ECHILD))
            /* the process has already been reaped earlier */
            {}
        else
        if (res == -1)
            /* handle an error */
            error_handling(res, __FILE__, __LINE__, "waitpid");
        else
        if (res == item->pid)
            /* we've just reaped the process */
            {}
    } else
        /* there was an error in the `open_io_redirecton_files` function in
        the `launch_process`. The process doesn't exist */
        {}
}

static void clean_up_the_rest_of_the_list(
    execvp_cmd_line *item, void (*fptr)(const execvp_cmd_line *item)
)
{
    while (item) {
        execvp_cmd_line *tmp;
        if (fptr)
            /* wait_for_cmd_linde_item(item); */
            (*fptr)(item);
        tmp = item;
        item = item->next;
        free(tmp->arr);
        free(tmp);
    }
}

static void clean_up_cmdline_list_except_first_item(
    execvp_cmd_line *first, void (*fptr)(const execvp_cmd_line *item)
)
{
    if (fptr)
        /* wait_for_cmd_linde_item(first); */
        (*fptr)(first);
    clean_up_the_rest_of_the_list(first->next, fptr);
    first->next = NULL;
}

static void handle_zombies(cmd_lines_list *cmdline)
{
    if (!cmdline->background_execution) {
        /* temporarilly turn off the `SIGCHLD` signal desposition */
        set_signal_disposition(SIGCHLD, SIG_DFL);
        /* wait for each process of the `cmdline` linked list */
        /* clean the linked list up, except for the first item */
        clean_up_cmdline_list_except_first_item(
            cmdline->first, &wait_for_cmd_linde_item
        );
        /* turn the `SIGCHLD` signal desposition back on */
        set_signal_disposition(SIGCHLD, handle_background_zombie_process);
    } else
        /* clean up the `cmdline` linked list, except for the first item */
        /* zombie processes will be reaped by `SIGCHLD` signal handler func */
        clean_up_cmdline_list_except_first_item(cmdline->first, NULL);
}

static int open_io_redirecton_files(
    const execvp_cmd_line *cmdline, int *fd_input, int *fd_output
)
{
    const char *input_file = cmdline->input.redirection_file;
    const char *output_overwrite_file =
        cmdline->output_overwrite.redirection_file;
    const char *output_append_file =
        cmdline->output_append.redirection_file;

    if (cmdline->input.redirection) {
        *fd_input = open(input_file, O_RDONLY);
        if ((*fd_input == -1) && (errno == ENOENT)) {
            fprintf(stderr, ERR_NO_SUCH_FILE, input_file);
            return -1;
        }
    }
    if (cmdline->output_overwrite.redirection)
        *fd_output = open(output_overwrite_file, O_WRONLY|O_CREAT|O_TRUNC,0666);
    if (cmdline->output_append.redirection)
        *fd_output = open(output_append_file, O_WRONLY|O_CREAT|O_APPEND, 0666);
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
    const execvp_cmd_line *cmdline, int fd_input, int fd_output
)
{
    int res;
    if (cmdline->input.redirection) {
        res = dup2(fd_input, 0);
        error_handling(res, __FILE__, __LINE__, "dup2");
    }
    if (cmdline->output_overwrite.redirection ||
        cmdline->output_append.redirection)
    {
        res = dup2(fd_output, 1);
        error_handling(res, __FILE__, __LINE__, "dup2");
    }
    close_io_redirection_files(fd_input, fd_output);
}

static void close_and_free_all_pipes(pipeline_item *curr)
{
    while (curr) {
        pipeline_item *tmp = curr;
        curr = curr->next;
        close(tmp->fd[0]);
        close(tmp->fd[1]);
        free(tmp);
    }
}

static void close_other_pipes_file_descriptors(
    const pipeline_item *prev, const pipeline_item *next,
    const pipeline_item *curr
)
{
    for (; curr; curr= curr->next) {
        if ((curr == prev) || (curr == next))
            continue;
        close(curr->fd[0]);
        close(curr->fd[1]);
    }
}

static void set_up_pipeline(
    const pipeline_item *prev, const pipeline_item *next,
    const pipeline_item *first
)
{
    int res;
    if (prev || next) {
        close_other_pipes_file_descriptors(prev, next, first);
        /* configure our process's `prev` and `next` pipes */
        if (prev) {
            /* we read from the `prev` pipe */
            close(prev->fd[1]);
            res = dup2(prev->fd[0], 0);
            error_handling(res, __FILE__, __LINE__, "dup2");
            close(prev->fd[0]);
        }
        if (next) {
            /* we write into the `next` pipe */
            close(next->fd[0]);
            res = dup2(next->fd[1], 1);
            error_handling(res, __FILE__, __LINE__, "dup2");
            close(next->fd[1]);
        }
    }
}

static void set_up_and_exec_child(
    const execvp_cmd_line *cmdline, int fd_input, int fd_output,
    const pipeline_item *prev_pipe, const pipeline_item *next_pipe,
    const pipeline_item *first_pipe
)
{
    set_up_pipeline(prev_pipe, next_pipe, first_pipe);
    io_redirection(cmdline, fd_input, fd_output);
    if (cmdline->arr[0]) {
        execvp(cmdline->arr[0], cmdline->arr);
        fprintf(
            stderr, "%s, %d, %s: %s:",
            __FILE__, __LINE__, "execvp", cmdline->arr[0]
        );
        perror("");
        fflush(stderr);
    }
    _exit(1);
}

static void launch_process(execvp_cmd_line *cmdline, pipeline_item **first_pipe)
{
    pipeline_item *prev_pipe = NULL, *next_pipe = *first_pipe;
    while (true) {
        int fd_input = 0, fd_output = 0, res;
        res = open_io_redirecton_files(cmdline, &fd_input, &fd_output);
        if (res == -1)
            goto exit;
        fflush(stderr);
        cmdline->pid = fork();
        error_handling(cmdline->pid, __FILE__, __LINE__, "fork");
        if (cmdline->pid == 0) {
            set_up_and_exec_child(
                cmdline, fd_input, fd_output, prev_pipe, next_pipe, *first_pipe
            );
        }
        close_io_redirection_files(fd_input, fd_output);
        cmdline = cmdline->next;
        if (!cmdline)
            break;
        prev_pipe = next_pipe;
        next_pipe = next_pipe->next;
    }
    exit:
    close_and_free_all_pipes(*first_pipe);
    *first_pipe = NULL;
}
#endif

void execute_command(string *str)
{
#if defined(PRINT_TOKENS_MODE)
    print_list_of_words(&str->words_list);
#elif defined(EXEC_MODE)
    error_code err = 0;
    if (words_list_is_empty(str))
        return;
    err = transform_words_list_into_cmd_line_arr(str);
    if (err) {
        print_error(err);
        return;
    }
    if (change_dir_command(str->cmd_line.last)) {
        handle_change_dir_command(str->cmd_line.last);
        return;
    }
    launch_process(str->cmd_line.first, &str->pipeline.first);
    handle_zombies(&str->cmd_line);
#endif
}
