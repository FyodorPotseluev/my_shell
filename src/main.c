/* main.c */

#if !defined(EXEC_MODE) && !defined(PRINT_TOKENS_MODE)
#error Please define either EXEC_MODE or PRINT_TOKENS_MODE
#endif

#include "str_parsing.h"
#include "constants.h"
#include "zombie_handling.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void init_str(string *str)
{
    string init_str = {
        false, false, false, false, 0, no_error,
        { NULL, 0, init_tmp_wrd_arr_len },
        { NULL, NULL, 1 },
        { NULL, init_cmd_line_arr_len }
    };
    init_str.tmp_wrd.arr = malloc(init_str.tmp_wrd.arr_len * sizeof(char));
    init_str.cmd_line.arr = malloc(init_str.cmd_line.arr_len * sizeof(char*));
    *str = init_str;
}

void free_memory(string *str)
{
    free_list_of_words(&str->words_list);
    free(str->tmp_wrd.arr);
    str->tmp_wrd.arr = NULL;
    free(str->cmd_line.arr);
    str->cmd_line.arr = NULL;
}

int main()
{
    string str;
    init_str(&str);
    set_signal_disposition(SIGCHLD, handle_background_zombie_process);
    printf("> ");
    while ((str.c=getchar_signal_protected()) != EOF) {
        process_character(&str);
        if (str.str_ended)
            process_end_of_string(&str);
    }
    free_memory(&str);
    printf("^D\n");
    return 0;
}
