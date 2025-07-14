/* reprogram_terminal.h */

#ifndef REPROGRAM_TERMINAL_H_INCLUDED
#define REPROGRAM_TERMINAL_H_INCLUDED

void reprogram_terminal_attributes();

void restore_default_terminal_attributes();

/* Reprogram and restore the terminal driver settings. This allows us to
administer the current input display in the terminal. */

#endif
