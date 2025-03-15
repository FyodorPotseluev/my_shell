/* words_in_brackets.c */
/* getchar */ /* putchar */ /* printf */
/* while ((c=getchar()) != EOF) { */

#include <stdio.h>

enum boolean { false, true };

int main()
{
    int c;
    enum boolean spc = true;
    enum boolean word = false;
    while ((c=getchar()) != EOF) {
        switch (c) {
            case '\n':
            case '\t':
            case ' ':
                if (word)
                    putchar(')');
                word = false;
                spc = true;
                break;
            default:
                if (spc)
                    putchar('(');
                word = true;
                spc = false;
        }
        putchar(c);
    }
    return 0;
}
