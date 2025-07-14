/* shut_down_my_shell.c */

#include "shut_down_my_shell.h"
#include "reprogram_terminal.h"
#include <stdio.h>
#include <stdlib.h>
#if defined(LOG)
#  include "handle_err.h"
#  include <unistd.h>

extern int log_fd;
#endif

void shut_down_my_shell(
    type_input *input, type_tokenizer *tknzer, type_tst *path_tree
)
{
#if defined(LOG)
    int res = close(log_fd);
    error_handling(res, __FILE__, __LINE__, "close");
#endif
    restore_default_terminal_attributes();
    if (input->end_idx != 0)
        printf("\n> ");
    free_tst(path_tree);
    free(input->str);
    free_tokenizer(tknzer);
    printf("^D\n");
    exit(0);
}
