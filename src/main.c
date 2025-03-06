/* main.c */

#if !defined(EXEC_MODE) && !defined(PRINT_TOKENS_MODE)
#error Please define either EXEC_MODE or PRINT_TOKENS_MODE
#endif

#include "str_parsing.h"
#include "constants.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
    string str = {
        false, false, false, false, 0, no_error,
        { NULL, 0, init_tmp_wrd_arr_len },
        { NULL, NULL, 1 },
        { NULL, init_cmd_line_arr_len }
    };
    str.tmp_wrd.arr = malloc(str.tmp_wrd.arr_len * sizeof(char));
    str.cmd_line.arr = malloc(str.cmd_line.arr_len * sizeof(char*));
    printf("> ");
    while ((str.c = getchar()) != EOF) {
        process_character(&str);
        if (str.str_ended)
            process_end_of_string(&str);
    }
    free_list_of_words(&str.words_list);
    free(str.tmp_wrd.arr);
    str.tmp_wrd.arr = NULL;
    free(str.cmd_line.arr);
    str.cmd_line.arr = NULL;
    printf("^D\n");
    return 0;
}
