/* zombie_handling.h */

#ifndef ZOMBIE_HANDLING_H_INCLUDED
#define ZOMBIE_HANDLING_H_INCLUDED

void handle_background_zombie_process(int sig_num);

void set_signal_disposition(int signum, void (*handler)(int));

int getchar_signal_protected();

#endif
