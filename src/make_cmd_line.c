/* make_cmd_line.c */

#include "make_cmd_line.h"
#include "handle_err.h"
#include <stdlib.h>
#include <unistd.h>

typedef enum tag_handle_separator_status {
    error = -1,
    move_on,
    completed
} handle_separator_status;

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
    const execvp_cmd_line *cmdline, const word_item *curr_word
)
{
    bool the_word_is_separator = (curr_word->separator_val != none);

    bool io_redirection_waiting_for_file =
        (cmdline->input.waiting_for_file ||
        cmdline->output_overwrite.waiting_for_file ||
        cmdline->output_append.waiting_for_file);

    return (the_word_is_separator && io_redirection_waiting_for_file);
}

static bool second_simple_word_right_after_io_redirecton(
    const execvp_cmd_line *cmdline, const word_item *curr_word
)
{
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
    word_item *curr_word, int idx, bool *next_step
)
{
    stream->redirection_file = curr_word->word;
    curr_word->word = NULL;
    stream->waiting_for_file = false;
    cmdline->arr[idx] = NULL;
    *next_step = true;
}

static handle_separator_status appoint_io_redirect_file(
    execvp_cmd_line *cmdline, word_item *curr_word,
    int idx, bool *next_step
)
{
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

static handle_separator_status start_background_execution(
    type_cmd_line *cmdline, word_item *curr_word, int idx, bool *next_step
)
{
    if (curr_word->separator_val == background_operator) {
        cmdline->last->arr[idx] = NULL;
        cmdline->background_execution = true;
        *next_step = true;
        return completed;
    } else
        return move_on;
}

void add_new_cmd_line_item(type_cmd_line *cmdline)
{
    /* add item at the end of `cmd_line` link list */
    cmdline->last->next = malloc(sizeof(execvp_cmd_line));
    cmdline->last = cmdline->last->next;
    init_cmd_line_item(cmdline->last);
}

void add_new_pipeline_item(type_cmd_line *cmdline)
{
    /* add `pipeline_item` at the beginning of `pipe` linked list */
    int res;
    pipeline_item *tmp = malloc(sizeof(pipeline_item));
    res = pipe(tmp->fd);
    error_handling(res, __FILE__, __LINE__, "pipe");
    tmp->next = cmdline->pipe;
    cmdline->pipe = tmp;
}

static handle_separator_status split_cmdline_and_add_pipeline(
    type_cmd_line *cmdline, word_item *curr_word, int *idx, bool *next_step
)
{
    if (curr_word->separator_val == pipe_operator) {
        cmdline->last->arr[*idx] = NULL;
        add_new_cmd_line_item(cmdline);
        add_new_pipeline_item(cmdline);
        *idx = -1;          /* on the next iteration it will be 0 */
        *next_step = true;
        return completed;
    } else
        return move_on;
}

static handle_separator_status toggle_stream_redirection(
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

static handle_separator_status toggle_io_redirection(
    execvp_cmd_line *cmdline, word_item *curr_word, int idx, bool *next_step
)
{
    handle_separator_status res;
    if (curr_word->separator_val == input_redirection) {
        bool error_condition = (cmdline->input.redirection);
        res = toggle_stream_redirection(
            &cmdline->input, cmdline, error_condition, idx, next_step
        );
        return res;
    } else
    if (curr_word->separator_val == output_redirection) {
        bool error_condition = (
            cmdline->output_overwrite.redirection ||
            cmdline->output_append.redirection
        );
        res = toggle_stream_redirection(
            &cmdline->output_overwrite, cmdline, error_condition, idx, next_step
        );
        return res;
    } else
    if (curr_word->separator_val == output_append_redirection) {
        bool error_condition = (
            cmdline->output_overwrite.redirection ||
            cmdline->output_append.redirection
        );
        res = toggle_stream_redirection(
            &cmdline->output_append, cmdline, error_condition, idx, next_step
        );
        return res;
    } else
        return move_on;
}

static error_code handle_possible_separator(
    type_cmd_line *cmdline, word_item *curr_word, int *idx, bool *next_step
)
{
    handle_separator_status status;
    if (cmdline->background_execution)
        return background_operator_not_in_the_end_of_str;
    if (pipe_operator_at_start_of_cmdline(curr_word, *idx))
        return pipe_operator_at_start_of_str;
    if (separator_right_after_io_redirecton(cmdline->last, curr_word))
        return separator_right_after_input_or_output_redirection;
    if (second_simple_word_right_after_io_redirecton(cmdline->last, curr_word))
        return second_simple_word_right_after_input_or_output_redirecton;
    status = split_cmdline_and_add_pipeline(cmdline, curr_word, idx, next_step);
    if (status == completed)
        return no_error;
    status = appoint_io_redirect_file(cmdline->last, curr_word, *idx,next_step);
    if (status == completed)
        return no_error;
    status = start_background_execution(cmdline, curr_word, *idx, next_step);
    if (status == completed)
        return no_error;
    status = toggle_io_redirection(cmdline->last, curr_word, *idx, next_step);
    if (status == completed)
        return no_error;
    if (status == error)
        return input_or_output_separator_used_in_line_twice;
    if (curr_word->separator_val != none)
        return not_implemented_feature;
    return no_error;
}

error_code make_cmd_line(type_cmd_line *cmdline, type_tokenizer *tknzer)
{
    word_item *p = tknzer->words_list.first;
    int i = 0;
    for (;; p=p->next, i++) {
        if (cmdline->last->arr_len == i)
            increase_cmd_line_array_length(cmdline->last);
        if (!p) {
            cmdline->last->arr[i] = NULL;
            break;
        } else {
            bool next_step = false;
            error_code err;
            err = handle_possible_separator(cmdline, p, &i, &next_step);
            if (err)
                return err;
            if (next_step)
                continue;
            cmdline->last->arr[i] = p->word;
            p->word = NULL;
        }
    }
    return 0;
}
