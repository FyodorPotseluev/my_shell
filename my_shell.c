/* my_shell.c */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum consts {
    initial_word_size = 8
};

typedef struct tag_word_item {
    char *word;
    struct tag_word_item *next;
} word_item;

void free_list_of_words(word_item *item)
{
    if (!item)
        return;
    free_list_of_words(item->next);
    free(item->word);
    free(item);
}

void print_list_of_words(const word_item *item)
{
    if (!item)
        return;
    print_list_of_words(item->next);
    printf("[%s]\n", item->word);
}

void switch_record_space_symbols(bool *record_spaces)
{
    if (*record_spaces)
        *record_spaces = false;
    else
        *record_spaces = true;
}

void add_empty_item_to_list_of_words(word_item **item)
{
    word_item *tmp = malloc(sizeof(word_item));
    tmp->word = NULL;
    tmp->next = *item;
    *item = tmp;
}

char *my_recalloc(char *old_short_str, int new_size)
{
    int old_size = new_size / 2;
    char *new_long_str = calloc(new_size, sizeof(char));
    memcpy(new_long_str, old_short_str, old_size);
    free(old_short_str);
    return new_long_str;
}

void add_character_to_word(
    word_item **item, int *curr_idx, bool *end_word, char c
)
{
    static int word_size;
    char *str;
    *end_word = false;
    if (!*item)
        add_empty_item_to_list_of_words(item);
    if (!(*item)->word) {
        word_size = initial_word_size;
        (*item)->word = calloc(word_size, sizeof(char));
        *curr_idx = 0;
    }
    if (*curr_idx == word_size-1) {
        word_size *= 2;
        (*item)->word = my_recalloc((*item)->word, word_size);
    }
    str = (*item)->word;
    str[*curr_idx] = c;
    (*curr_idx)++;
}

void process_space_symbol(
    word_item **item, int *curr_idx, bool *record_space_symbols,
    bool *end_word, char c
)
{
    if (*record_space_symbols)
        add_character_to_word(item, curr_idx, end_word, c);
    else
    if (!*end_word) {
        add_empty_item_to_list_of_words(item);
        *end_word = true;
    }
}

void add_char_to_list_of_words(
    word_item **item, int *curr_idx, bool *record_space_symbols,
    bool *end_word, char c
)
{
    switch (c) {
        case ('\t'):
        case (' '):
            process_space_symbol(
                item, curr_idx, record_space_symbols, end_word, c
            );
            break;
        case ('"'):
            switch_record_space_symbols(record_space_symbols);
            break;
        default:
            add_character_to_word(item, curr_idx, end_word, c);
    }
}

int main()
{
    word_item *words_list = NULL;
    int c;
    for (;;) {
        bool record_space_symbols = false, end_word = false;
        int curr_idx = 0;
        printf("> ");
        while ((c = getchar()) != '\n') {
            if (c == EOF) {
                free_list_of_words(words_list);
                printf("^D\n");
                return 0;
            }
            add_char_to_list_of_words(
                &words_list, &curr_idx, &record_space_symbols, &end_word, c
            );
        }
        if (record_space_symbols)
            printf("Error: unmatched quotes\n");
        else
            print_list_of_words(words_list);
        free_list_of_words(words_list);
        words_list = NULL;
    }
    return 0;
}
