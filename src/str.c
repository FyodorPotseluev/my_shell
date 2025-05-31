/* str.c */

#include "str.h"
#include <stdlib.h>

bool words_list_is_empty(const string *str)
{
    return (str->words_list.first) ? false : true;
}

void init_str(string *str)
{
    str->word_ended = false;
    str->str_ended = false;
    str->quotation = false;
    str->char_escaping = false;
    str->c = 0;
    str->err_code = no_error;
    str->tmp_wrd.arr_len = init_tmp_wrd_arr_len;
    str->tmp_wrd.arr = malloc(str->tmp_wrd.arr_len * sizeof(char));
    str->tmp_wrd.idx = 0;
    str->words_list.first = NULL;
    str->words_list.last = NULL;
}

void reset_str_variables(string *str)
{
    str->word_ended = false;
    str->str_ended = false;
    str->quotation = false;
    str->char_escaping = false;
    str->err_code = no_error;
    str->tmp_wrd.idx = 0;
}

void free_list_of_words(curr_str_words_list *link_list)
{
    word_item *p = link_list->first;
    while (p) {
        word_item *tmp = p;
        p = p->next;
        if (tmp->word)
            /* free_list_of_words_strings(tmp->word) */
            free(tmp->word);
        free(tmp);
    }
    link_list->first = NULL;
    link_list->last = NULL;
}

void free_str(string *str)
{
    free_list_of_words(&str->words_list);
    free(str->tmp_wrd.arr);
    str->tmp_wrd.arr = NULL;
}
