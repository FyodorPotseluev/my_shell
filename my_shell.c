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

void free_list_of_words(word_item **item)
{
    if (!*item)
        return;
    free_list_of_words(&(*item)->next);
    free((*item)->word);
    free(*item);
    *item = NULL;
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

void switch_record_space_symbols(bool *record_spaces)
{
    if (input->record_space_symbols)
        input->record_space_symbols = false;
    else
        input->record_space_symbols = true;
}

void add_empty_item_to_list_of_words(word_item **item)
{
    word_item *tmp = malloc(sizeof(word_item));
    tmp->word = NULL;
    tmp->next = *item;
    *item = tmp;
}

char *my_realloc(char *old_short_str, int new_size)
{
    int old_size = new_size / 2;
    char *new_long_str = malloc(new_size * sizeof(char));
    memcpy(new_long_str, old_short_str, old_size);
    free(old_short_str);
    return new_long_str;
}

void add_character_to_word(
    word_item **item, dynamic_char_arr *tmp_wrd, bool *word_ended, char c
)
{
    *word_ended = false;
    if (!*item)
        add_empty_item_to_list_of_words(item);
    if (tmp_wrd->idx == tmp_wrd->size-1) {
        tmp_wrd->size *= 2;
        tmp_wrd->arr = my_realloc(tmp_wrd->arr, tmp_wrd->size);
    }
    tmp_wrd->arr[tmp_wrd->idx] = c;
    tmp_wrd->idx++;
}

void process_end_of_word(word_item *item, dynamic_char_arr *tmp_wrd)
{
    tmp_wrd->arr[tmp_wrd->idx] = '\0';
    if (item) {
        item->word = malloc((tmp_wrd->idx + 1) * sizeof(char));
        strcpy(item->word, tmp_wrd->arr);
    }
    tmp_wrd->idx = 0;
}

void prepare_to_read_next_string(
    bool *record_space_symbols, bool *word_ended,
    bool *str_ended, dynamic_char_arr *tmp_wrd
)
{
    *record_space_symbols = false;
    *word_ended = false;
    *str_ended = false;
    tmp_wrd->idx = 0;
    printf("> ");
}

void process_end_of_string(
    word_item **words_list, bool *record_space_symbols,
    bool *word_ended, bool *str_ended, dynamic_char_arr *tmp_wrd
)
{
    if (*record_space_symbols)
        printf("Error: unmatched quotes\n");
    else
        execute_command(*words_list);
    free_list_of_words(words_list);
    prepare_to_read_next_string(
        record_space_symbols, word_ended, str_ended, tmp_wrd
    );
}

void process_space_symbol(
    word_item **item, dynamic_char_arr *tmp_wrd, bool record_space_symbols,
    bool *word_ended, char c
)
{
    if (record_space_symbols)
        add_character_to_word(item, tmp_wrd, word_ended, c);
    else
    if (!*word_ended && *item) {
        process_end_of_word(*item, tmp_wrd);
        add_empty_item_to_list_of_words(item);
        *word_ended = true;
    }
}

void process_character(
    word_item **item, dynamic_char_arr *tmp_wrd, bool *record_space_symbols,
    bool *word_ended, bool *str_ended, char c
)
{
    switch (input->c) {
        case ('\t'):
        case (' '):
            process_space_symbol(
                item, tmp_wrd, *record_space_symbols, word_ended, c
            );
            break;
        case ('\n'):
            process_end_of_word(*item, tmp_wrd);
            *word_ended = true;
            *str_ended = true;
            break;
        case ('"'):
            process_quote_sign_symbol(item, input);
            break;
        case ('\n'):
            input->stop_reading_curr_str = true;
            break;
        default:
            add_character_to_word(item, tmp_wrd, word_ended, c);
    }
    else
        print_list_of_words(words_list);
}

int main()
{
    word_item *words_list = NULL;
    dynamic_char_arr tmp_wrd = { NULL, 0, init_tmp_wrd_arr_size };
    int c;
    bool record_space_symbols, word_ended, str_ended;
    tmp_wrd.arr = malloc(tmp_wrd.size * sizeof(char));
    prepare_to_read_next_string(
        &record_space_symbols, &word_ended, &str_ended, &tmp_wrd
    );
    while ((c = getchar()) != EOF) {
        process_character(
            &words_list, &tmp_wrd, &record_space_symbols,
            &word_ended, &str_ended, c
        );
        if (str_ended)
            process_end_of_string(
                &words_list, &record_space_symbols,
                &word_ended, &str_ended, &tmp_wrd
            );
    }
    free_list_of_words(&words_list);
    free(tmp_wrd.arr);
    printf("^D\n");
    return 0;
}
