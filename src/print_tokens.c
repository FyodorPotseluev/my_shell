/* print_tokens.c */

#include "print_tokens.h"
#include <stdio.h>

static void print_separator_value(separator_type separator_val)
{
    switch (separator_val) {
        case (none):
            fprintf(stderr, "my_shell: Error: something went wrong :/\n");
            break;
        case (background_operator):
            printf("[background_operator]\n");
            break;
        case (and_operator):
            printf("[and_operator]\n");
            break;
        case (output_redirection):
            printf("[output_redirection]\n");
            break;
        case (output_append_redirection):
            printf("[output_append_redirection]\n");
            break;
        case (pipe_operator):
            printf("[pipe_operator]\n");
            break;
        case (or_operator):
            printf("[or_operator]\n");
            break;
        case (input_redirection):
            printf("[input_redirection]\n");
            break;
        case (command_separator):
            printf("[command_separator]\n");
            break;
        case (open_parenthesis):
            printf("[open_parenthesis]\n");
            break;
        case (close_parenthesis):
            printf("[close_parenthesis]\n");
    }
}

void print_tokens(const curr_str_words_list *words_list)
{
    word_item *p = words_list->first;
    while (p) {
        if (p->separator_val != none)
            print_separator_value(p->separator_val);
        else
            printf("[%s]\n", p->word ? p->word : "");
        p = p->next;
    }
}
