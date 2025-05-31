/* cmd_line.c */

#include "cmd_line.h"
#include <stdlib.h>

bool cmd_line_is_empty(const cmd_line *cmdline)
{
    return (cmdline->first->arr[0]) ? false : true;
}

static void free_pipes_link_list(pipeline_item *p)
{
    while (p) {
        pipeline_item *tmp = p;
        p = p->next;
        free(tmp);
    }
}

static void free_cmd_line_strs_and_io_files(execvp_cmd_line *p)
{
    int i;
    for (i=0; p->arr[i]; i++) {
        free(p->arr[i]);
        p->arr[i] = NULL;
    }
    if (p->input.redirection_file) {
        free(p->input.redirection_file);
        p->input.redirection_file = NULL;
    }
    if (p->output_overwrite.redirection_file) {
        free(p->output_overwrite.redirection_file);
        p->output_overwrite.redirection_file = NULL;
    }
    else
    if (p->output_append.redirection_file) {
        free(p->output_append.redirection_file);
        p->output_append.redirection_file = NULL;
    }
}

static void free_cmd_line_link_list(execvp_cmd_line *p)
{
    while (p) {
        execvp_cmd_line *tmp;
        free_cmd_line_strs_and_io_files(p);
        /* free the array that stored the `cmd_line` strings */
        free(p->arr);
        tmp = p;
        p = p->next;
        /* free the entire `cmd_line` item */
        free(tmp);
    }
}

void free_cmd_line(cmd_line *cmdline)
{
    free_cmd_line_link_list(cmdline->first);
    cmdline->first = NULL;
    cmdline->last = NULL;
    free_pipes_link_list(cmdline->pipe);
    cmdline->pipe = NULL;
}

static void init_io_status(io_status *io_stat)
{
    io_stat->redirection = false;
    io_stat->waiting_for_file = false;
    io_stat->redirection_file = NULL;
}

void init_cmd_line_item(execvp_cmd_line *item)
{
    item->arr_len = init_cmd_line_arr_len;
    item->arr = malloc(item->arr_len * sizeof(char*));
    item->pid = 0;
    init_io_status(&item->input);
    init_io_status(&item->output_overwrite);
    init_io_status(&item->output_append);
    item->next = NULL;
}

void init_cmd_line(cmd_line *cmdline)
{
    cmdline->first = malloc(sizeof(execvp_cmd_line));
    init_cmd_line_item(cmdline->first);
    cmdline->last = cmdline->first;
    cmdline->background_execution = false;
    cmdline->err_code = no_error;
    cmdline->pipe = NULL;
}
