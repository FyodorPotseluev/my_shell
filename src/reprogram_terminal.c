/* reprogram_terminal.c */

#include "reprogram_terminal.h"
#include "handle_err.h"
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios default_termios;

void reprogram_terminal_attributes()
{
    int res;
    struct termios st;
    if (!isatty(0))
        /* the sdandard input was redirected */
        /* we don't need to reprogram the terminal */
        return;
    res = tcgetattr(0, &default_termios);
    error_handling(res, __FILE__, __LINE__, "tcgetattr");
    st = default_termios;
    /* switch off `ICANON` - immediately receive all incoming bytes */
    /* switch off `ECHO` - turn off local echo */
    st.c_lflag &= ~(ICANON | ECHO);
    /* get input byte immediately or wait for it if don't have it yet*/
    st.c_cc[VMIN] = 1;
    st.c_cc[VTIME] = 0;
    res = tcsetattr(0, TCSANOW, &st);
    error_handling(res, __FILE__, __LINE__, "tcsetattr");
}

void restore_default_terminal_attributes()
{
    int res;
    if (!isatty(0))
        /* the sdandard input was redirected */
        /* we don't need to reprogram the terminal */
        return;
    res = tcsetattr(0, TCSANOW, &default_termios);
    if (res == -1) {
        perror("restore_tcsetattr");
        exit(1);
    }
}
