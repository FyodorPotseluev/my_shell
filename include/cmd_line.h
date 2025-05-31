/* cmd_line.h */

#ifndef CMD_LINE_H_INCLUDED
#define CMD_LINE_H_INCLUDED

#include "err_code.h"
#include <stdbool.h>

enum cmd_line_h_consts {
    init_cmd_line_arr_len = 8
};

typedef struct tag_io_status {
    bool redirection;
    bool waiting_for_file;
    char *redirection_file;
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

typedef struct tag_pipeline_item {
    int fd[2];
    struct tag_pipeline_item *next;
} pipeline_item;

typedef struct tag_cmd_line {
    execvp_cmd_line *first;
    execvp_cmd_line *last;
    bool background_execution;
    error_code err_code;
    /* contains the pipes linking processes into a pipeline */
    pipeline_item *pipe;
} cmd_line;

void init_cmd_line_item(execvp_cmd_line *item);
/*
    Initializes a single `execvp_cmd_line` linked list item.
RECEIVES:
    - `item` address of `execvp_cmd_line` linked list item to be initialized */

void init_cmd_line(cmd_line *cmdline);
/*
    Initializes complete `cmd_line` structure.
RECEIVES:
    - `cmdline` address of `cmd_line` structure */

bool cmd_line_is_empty(const cmd_line *cmdline);
/*
    Checks if the `cmd_line` structure contains no `execvp_cmd_line` items.
RECEIVES:
    - `cmdline` address of `cmd_line` structure;
RETURNES:
    - `true` if empty, `false` otherwise */

void free_cmd_line(cmd_line *cmdline);
/*
    Frees memory, dynamically allocated for the `cmd_line` structure;
RECEIVES:
    - `cmdline` address of `cmd_line` structure */

#endif
