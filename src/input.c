/* input.c */

#include "input.h"
#include <stdlib.h>

void reset_input(type_input *input)
{
    input->end_idx = 0;
    input->cur_idx = 0;
    input->wrd_idx = 0;
    input->_1st_wrd_start_idx = -1;
    input->_1st_wrd_end_idx = -1;
    input->num_of_tab_key_presses = 0;
}

void init_input(type_input *input)
{
    input->str_len = init_input_str_len;
    input->str = malloc(input->str_len*sizeof(char));
    reset_input(input);
}
