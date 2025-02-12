/* my_shell.c */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum consts {
    init_tmp_wrd_arr_size = 16
};

typedef struct tag_struct_input {
    char c;
    bool record_space_symbols;
    bool end_of_word;
    bool character_escaping;
    bool stop_reading_curr_str;
    int curr_idx;
} struct_input;

typedef struct tag_word_item {
    char *word;
    struct tag_word_item *next;
} word_item;

typedef struct tag_dynamic_char_arr {
    char *arr;
    int idx;
    int size;
} dynamic_char_arr;

typedef struct tag_string {
    bool word_ended, str_ended, include_spaces;
    int c;
    word_item *words_list;
    dynamic_char_arr tmp_wrd;
} string;

void free_list_of_words(word_item *item)
{
    if (!item)
        return;
    free_list_of_words(item->next);
    free(item->word);
    free(item);
}

void print_list_of_words_reqursive_call(const word_item *item)
{
    if (!item)
        return;
    print_list_of_words_reqursive_call(item->next);
    printf("[%s]\n", item->word);
}

void print_list_of_words(const word_item *words_list)
{
    if (words_list)
        print_list_of_words_reqursive_call(words_list);
}

void execute_command(const word_item *words_list)
{
    print_list_of_words(words_list);
}

void toggle_include_spaces(string *str)
{
    if (str->include_spaces)
        str->include_spaces = false;
    else
        str->include_spaces = true;
}

void add_empty_item_to_list_of_words(string *str)
{
    word_item *tmp = malloc(sizeof(word_item));
    tmp->word = NULL;
    tmp->next = str->words_list;
    str->words_list = tmp;
}

char *my_realloc(char *old_short_str, int new_size)
{
    int old_size = new_size / 2;
    char *new_long_str = malloc(new_size * sizeof(char));
    memcpy(new_long_str, old_short_str, old_size);
    free(old_short_str);
    return new_long_str;
}

void add_character_to_word(string *str)
{
    str->word_ended = false;
    /* if we have empty list of words or finished last word (if the last word
    isn't finished, it's still forming in the `tmp_wrd` array and thus
    `str->words_list->word` is NULL), add new item to the list of words */
    if ((!str->words_list) || (str->words_list->word))
        add_empty_item_to_list_of_words(str);
    if (str->tmp_wrd.idx == str->tmp_wrd.size-1) {
        int new_size = str->tmp_wrd.size * 2;
        str->tmp_wrd.arr = my_realloc(str->tmp_wrd.arr, new_size);
    }
    str->tmp_wrd.arr[str->tmp_wrd.idx] = str->c;
    (str->tmp_wrd.idx)++;
}

void process_end_of_word(string *str)
{
    str->tmp_wrd.arr[str->tmp_wrd.idx] = '\0';
    if (str->words_list) {
        str->words_list->word = malloc((str->tmp_wrd.idx + 1) * sizeof(char));
        strcpy(str->words_list->word, str->tmp_wrd.arr);
    }
    str->tmp_wrd.idx = 0;
}

void reset_str_variables(string *str)
{
    str->include_spaces = false;
    str->word_ended = false;
    str->str_ended = false;
    str->tmp_wrd.idx = 0;
}

void process_end_of_string(string *str)
{
    if (str->include_spaces)
        printf("Error: unmatched quotes\n");
    else
        execute_command(str->words_list);
    free_list_of_words(str->words_list);
    str->words_list = NULL;
    reset_str_variables(str);
    printf("> ");
}

void complete_word(string *str)
{
    if (!str->word_ended && str->words_list) {
        process_end_of_word(str);
        str->word_ended = true;
    }
}

void process_space_character(string *str)
{
    if (str->include_spaces)
        add_character_to_word(str);
    else
        complete_word(str);
}

void process_character(string *str)
{
    switch (str->c) {
        case ('\t'):
        case (' '):
            process_space_character(str);
            break;
        case ('\n'):
            complete_word(str);
            str->str_ended = true;
            break;
        case ('"'):
            toggle_include_spaces(str);
            break;
        default:
            add_character_to_word(str);
    }
    else
        print_list_of_words(words_list);
}

int main()
{
    string str =
        { false, false, false, 0, NULL, { NULL, 0, init_tmp_wrd_arr_size } };
    str.tmp_wrd.arr = malloc(str.tmp_wrd.size * sizeof(char));
    printf("> ");
    while ((str.c = getchar()) != EOF) {
        process_character(&str);
        if (str.str_ended)
            process_end_of_string(&str);
    }
    free_list_of_words(str.words_list);
    str.words_list = NULL;
    free(str.tmp_wrd.arr);
    printf("^D\n");
    return 0;
}
