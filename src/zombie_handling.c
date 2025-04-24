/* zombie_handling.c */

#include "error_handling.h"
#include "zombie_handling.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static bool getchar_in_block_mode_was_interrupted_by_signal()
{
    return (errno == EINTR);
}

int getchar_signal_protected()
{
    while (true) {
        int chr = getchar();
        if (getchar_in_block_mode_was_interrupted_by_signal()) {
            errno = 0;
            continue;
        } else
            return chr;
    }
}

static void handle_errors(int res)
{
    char header[] = "`waitpid` failed in the `zombie_handling.c`: ";
    char err_msg_eintr[] =
        "WNOHANG was not set and an unblocked signal or a SIGCHLD was caught\n";
    char err_msg_einval[] = "the 3rd `waitpid` argument was invalid\n";
    char err_msg_unexpected[] = "unexpected `errno` value\n";
    if ((res == -1) && (errno != ECHILD)) {
        write(2, header, sizeof(header)-1);
        switch (errno) {
            case (EINTR):
                write(2, err_msg_eintr, sizeof(err_msg_eintr)-1);
                break;
            case (EINVAL):
                write(2, err_msg_einval, sizeof(err_msg_einval)-1);
                break;
            default:
                write(2, err_msg_unexpected, sizeof(err_msg_unexpected)-1);
        }
    }
}

void handle_background_zombie_process(int sig_num)
{
    int res, saved_errno = errno;
    (void)sig_num;
    errno = 0;
    do {
        res = waitpid(-1, NULL, WNOHANG);
        handle_errors(res);
    } while (res > 0);
    errno = saved_errno;
}

void set_signal_disposition(int signum, void (*handler)(int))
{
    int res;
    struct sigaction act;
    act.sa_handler = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    res = sigaction(signum, &act, NULL);
    error_handling(res, __FILE__, __LINE__, "sigaction");
}
