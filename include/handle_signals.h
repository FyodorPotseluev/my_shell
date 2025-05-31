/* handle_signals.h */

#ifndef HANDLE_SIGNALS_H_INCLUDED
#define HANDLE_SIGNALS_H_INCLUDED

void handle_background_zombie_process(int sig_num);
/*
    Signal handler for `SIGCHLD` to reap zombie processes.
RECEIVES:
    - `sig_num` signal number (`SIGCHLD`, the function won't be called for other
    received signals) */

void set_signal_disposition(int signum, void (*handler)(int));
/*
    Sets the signal handler function for the specified signal.
RECEIVES:
    - `sig_num` signal number to handle;
    - `handler` the address of the function to handle the signal */

int getchar_signal_protected();
/*
    Gets character from input with signal interruption protection.
RETURNES:
    - character read from input */

#endif
