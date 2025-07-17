/* execute_command.c */

#include "execute_command.h"
#include "handle_err.h"
#include "handle_signals.h"
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

extern char *environ;

#define ERR_NO_SUCH_FILE \
    "my_shell: %s: No such file or directory\n"
#define ERR_WRONG_CTRL_TERM_FD \
    "%s:%d: `tcsetpgrp` got NOT the control term fd of the calling process\n"

typedef struct tag_io {
    int fd_input, fd_output;
} type_io;

typedef struct tag_pipe {
    const pipeline_item *prev;
    const pipeline_item *next;
    const pipeline_item *first;
} type_pipe;

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

static void make_process_group_foreground(
    int pgid, const type_cmd_line *setup, const char *file_name, int line_num
)
{
    int res = tcsetpgrp(setup->session_control_terminal_fd, pgid);
    if (res != pgid && res > 1)
        fprintf(stderr, ERR_WRONG_CTRL_TERM_FD, file_name, line_num);
    error_handling(res, file_name, line_num, "tcsetpgrp");
}

static void handle_zombies(const type_cmd_line *cmdline)
{
    const execvp_cmd_line *p;
    if (!cmdline->background_execution) {
        /* temporarilly turn off the `SIGCHLD` signal desposition */
        set_signal_disposition(SIGCHLD, SIG_DFL);
        /* wait for each process of the `cmdline` linked list */
        for (p = cmdline->first; p; p = p->next)
            wait_for_cmd_linde_item(p);
        make_process_group_foreground(
            cmdline->my_shell_group_pgid, cmdline, __FILE__, __LINE__
        );
        /* turn the `SIGCHLD` signal desposition back on */
        set_signal_disposition(SIGCHLD, handle_background_zombie_process);
    }
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

static void close_all_pipes(const pipeline_item *curr)
{
    while (curr) {
        close(curr->fd[0]);
        close(curr->fd[1]);
        curr = curr->next;
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

static void execvp_error_handling(
    const char *cmd_name, const char *file_name, int line_number
)
{
    fprintf(
        stderr, "%s, %d, %s: %s:", file_name, line_number, "execvp", cmd_name
    );
    perror("");
    fflush(stderr);
    _exit(1);
}

static void add_process_to_existing_pipeline_process_group(int pgid)
{
    int res = setpgid(0, pgid);
    error_handling(res, __FILE__, __LINE__, "setpgid");
}

static void create_new_process_group()
{
    int res = setpgid(0, 0);
    error_handling(res, __FILE__, __LINE__, "setpgid");
}

static void set_up_and_exec_child(
   const execvp_cmd_line *cmdline, const type_io *io, const type_pipe *pipe
)
{
    set_up_pipeline(pipe->prev, pipe->next, pipe->first);
    io_redirection(cmdline, io->fd_input, io->fd_output);
    if (cmdline->arr[0]) {
        execvp(cmdline->arr[0], cmdline->arr);
        /* if we got here - `execvp` failed */
        execvp_error_handling(cmdline->arr[0], __FILE__, __LINE__);
    }
    _exit(0);
}

static bool new_process_group_doesnt_exist_yet(int pgid)
{
    return (pgid == 0) ? true : false;
}

static void manage_process_group(int pgid, const type_cmd_line *cmdline_setup)
{
    if (new_process_group_doesnt_exist_yet(pgid)) {
        create_new_process_group();
        if (!cmdline_setup->background_execution)
            make_process_group_foreground(
                getpgrp(), cmdline_setup, __FILE__, __LINE__
            );
    } else
        add_process_to_existing_pipeline_process_group(pgid);
}

static void launch_process(
    const type_cmd_line *cmdline_setup, execvp_cmd_line *cmdline,
    const pipeline_item *first_pipe
)
{
    int pgid = 0;
    type_pipe pipe = { NULL, first_pipe, first_pipe };
    while (true) {
        type_io io = { 0, 0 };
        int res = open_io_redirecton_files(cmdline, &io.fd_input, &io.fd_output);
        if (res == -1)
            goto exit;
        fflush(stderr);
        cmdline->pid = fork();
        error_handling(cmdline->pid, __FILE__, __LINE__, "fork");
        if (cmdline->pid == 0) {
            manage_process_group(pgid, cmdline_setup);
            set_up_and_exec_child(cmdline, &io, &pipe);
        }
        if (pgid == 0)
            pgid = cmdline->pid;
        close_io_redirection_files(io.fd_input, io.fd_output);
        cmdline = cmdline->next;
        if (!cmdline)
            break;
        pipe.prev = pipe.next;
        pipe.next = pipe.next->next;
    }
    exit:
    close_all_pipes(pipe.first);
}

void execute_command(type_cmd_line *cmdline)
{
    if (change_dir_command(cmdline->first)) {
        /* if we have `cd` command ran inside pipe */
        if (cmdline->first != cmdline->last) {
            close_all_pipes(cmdline->pipe);
            return;
        }
        handle_change_dir_command(cmdline->first);
        return;
    }
    launch_process(cmdline, cmdline->first, cmdline->pipe);
    handle_zombies(cmdline);
}
