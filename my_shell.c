/* my_shell.c */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum consts {
    init_tmp_wrd_arr_size = 16
};

typedef enum tag_error_code {
    ok, incorrect_char_escaping
} error_code;

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
    bool word_ended, str_ended, include_spaces, char_escaping;
    int c;
    error_code err_code;
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
    printf("[%s]\n", (item->word) ? (item->word) : "");
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
    /* if we haven't started to write the `tmp_wrd` yet */
    if (!str->tmp_wrd.idx)
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
    str->word_ended = false;
    str->str_ended = false;
    str->include_spaces = false;
    str->char_escaping = false;
    str->err_code = ok;
    str->tmp_wrd.idx = 0;
}

bool report_if_error(string *str)
{
    bool error = true;
    if (str->err_code == incorrect_char_escaping)
        printf("Error: only the characters `\"` and `\\` can be escaped\n");
    else
    /* check this error condition before last */
    /* the `stdin_cleanup` function, which could be called if previous errors
    were detected, could break the balance of quote signs */
    if (str->include_spaces)
        printf("Error: unmatched quotes\n");
    else
    /* check this error condition last */
    if (str->err_code == ok)
        error = false;
    return error;
}

void process_end_of_string(string *str)
{
    bool error = report_if_error(str);
    if (error)
        return;
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

void stdin_cleanup()
{
    int c;
    while ((c=getchar() != '\n') && (c != EOF))
        ;
}

void check_incorrect_character_escaping(string *str)
{
    if (str->char_escaping) {
        str->err_code = incorrect_char_escaping;
        str->str_ended = true;
        stdin_cleanup();
    }
}

void process_escaped_character(string *str)
{
    add_character_to_word(str);
    str->char_escaping = false;
}

void process_character_escape_symbol(string *str)
{
    if (str->char_escaping) {
        process_escaped_character(str);
    } else
        str->char_escaping = true;
}

void possible_case_of_adding_empty_word(string *str);

void process_quotation_mark_character(string *str)
{
    if (str->char_escaping) {
        process_escaped_character(str);
    } else {
        toggle_include_spaces(str);
        possible_case_of_adding_empty_word(str);
    }
}

void process_character(string *str)
{
    switch (str->c) {
        case ('\t'):
        case (' '):
            check_incorrect_character_escaping(str);
            process_space_character(str);
            break;
        case ('\n'):
            check_incorrect_character_escaping(str);
            complete_word(str);
            str->str_ended = true;
            break;
        case ('\\'):
            process_character_escape_symbol(str);
            break;
        case ('"'):
            process_quotation_mark_character(str);
            break;
        default:
            check_incorrect_character_escaping(str);
            add_character_to_word(str);
    }
}

void possible_case_of_adding_empty_word(string *str)
{
    if ((!str->words_list) || (str->word_ended)) {
        str->c = getchar();
        if (str->c == '"') {
            toggle_include_spaces(str);
            str->c = getchar();
            if (str->c == ' ' || str->c == '\t' || str->c == '\n')
                add_empty_item_to_list_of_words(str);
        }
        process_character(str);
    }
}

int main()
{
    string str = {
        false, false, false, false, 0, ok, NULL,
        { NULL, 0, init_tmp_wrd_arr_size }
    };
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
