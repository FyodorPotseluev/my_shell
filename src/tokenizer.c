/* tokenizer.c */

#include "tokenizer.h"
#include <stdlib.h>

bool words_list_is_empty(const type_tokenizer *tknzer)
{
    return (tknzer->words_list.first) ? false : true;
}

void init_tokenizer(type_tokenizer *tknzer)
{
    tknzer->word_ended = false;
    tknzer->str_ended = false;
    tknzer->quotation = false;
    tknzer->char_escaping = false;
    tknzer->c = 0;
    tknzer->err_code = no_error;
    tknzer->tmp_wrd.arr_len = init_tmp_wrd_arr_len;
    tknzer->tmp_wrd.arr = malloc(tknzer->tmp_wrd.arr_len * sizeof(char));
    tknzer->tmp_wrd.idx = 0;
    tknzer->words_list.first = NULL;
    tknzer->words_list.last = NULL;
}

void reset_tokenizer_variables(type_tokenizer *tknzer)
{
    tknzer->word_ended = false;
    tknzer->str_ended = false;
    tknzer->quotation = false;
    tknzer->char_escaping = false;
    tknzer->err_code = no_error;
    tknzer->tmp_wrd.idx = 0;
}

void free_list_of_words(curr_str_words_list *link_list)
{
    word_item *p = link_list->first;
    while (p) {
        word_item *tmp = p;
        p = p->next;
        if (tmp->word)
            free(tmp->word);
        free(tmp);
    }
    link_list->first = NULL;
    link_list->last = NULL;
}

void free_tokenizer(type_tokenizer *tknzer)
{
    free_list_of_words(&tknzer->words_list);
    free(tknzer->tmp_wrd.arr);
    tknzer->tmp_wrd.arr = NULL;
}
