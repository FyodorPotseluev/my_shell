/* input.h */

#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#include <stdbool.h>

enum input_consts {
    init_input_str_len   = 16,  /* the initial size of the `input.str` buffer */
    input_buf_size      = 256   /* the max size to read from `fd=0` at a time */
};

typedef struct struct_input {;
    char *str;
    int str_len, end_idx, cur_idx, wrd_idx;
    int _1st_wrd_start_idx, _1st_wrd_end_idx, num_of_tab_key_presses;
} type_input;
/*
    Contains information about the state of the input formed by an interactive
keyboard presses:
    - `str` the current state of the input buffer;
    - `str_len` the current size of the input buffer;
    - `end_idx` the index of the zero byte ending the input string;
    - `cur_idx` the index of the current cursor position inside the buffer;
    - `wrd_idx` if we're inside a word - the current index from the word start;
    - `_1st_wrd_start_idx` the index of the 1st word start;
    - `_1st_wrd_end_idx` the index of the 1st word end;
    - `num_of_tab_key_presses` the number the tab key has been pressed in a row
*/

void init_input(type_input *input);
/*
    Initialize the input variable before the 1st use
*/

void reset_input(type_input *input);
/*
    Reset the input variable values after we flushed previous input with '\n'
*/

#endif
