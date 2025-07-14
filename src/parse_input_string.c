/* parse_input_string.c */

#include "parse_input_string.h"
#include "handle_signals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

error_code there_is_a_parsing_error(const type_tokenizer *tknzer)
{
    if (tknzer->err_code == incorrect_char_escaping)
        return incorrect_char_escaping;
    else
    /* check this error condition before last */
    /* the `stdin_cleanup` function, which could be called if previous errors
    were detected, could break the balance of quote signs */
    if (tknzer->quotation)
        return unmatched_quotes;
    else
    /* check this error condition last */
    if (tknzer->err_code == no_error)
        return no_error;
    else {
        /* we should never get to this place in the code */
        fprintf(stderr, "Unexpected error: %s line %d\n", __FILE__, __LINE__);
        return tknzer->err_code;
    }
}

static void toggle_quotation(type_tokenizer *tknzer)
{
    if (tknzer->quotation)
        tknzer->quotation = false;
    else
        tknzer->quotation = true;
}

static void add_empty_item_to_list_of_words(curr_str_words_list *list)
{
    if (!list->first) {
        list->first = malloc(sizeof(word_item));
        list->last = list->first;
    } else {
        list->last->next = malloc(sizeof(word_item));
        list->last = list->last->next;
    }
    list->last->word = NULL;
    list->last->separator_val = none;
    list->last->next = NULL;
}

static void double_tmp_wrd_arr(curr_word_dynamic_char_arr *tmp_wrd)
{
    char *doubled_arr = malloc(tmp_wrd->arr_len*2 * sizeof(char));
    memcpy(doubled_arr, tmp_wrd->arr, tmp_wrd->arr_len);
    free(tmp_wrd->arr);
    tmp_wrd->arr = doubled_arr;
    tmp_wrd->arr_len *= 2;
}

static void add_character_to_word(type_tokenizer *tknzer, type_input *input)
{
    tknzer->word_ended = false;
    /* if we haven't started to write the `tmp_wrd` yet */
    if (tknzer->tmp_wrd.idx == 0)
        add_empty_item_to_list_of_words(&tknzer->words_list);
    if (tknzer->tmp_wrd.idx == tknzer->tmp_wrd.arr_len-1)
        double_tmp_wrd_arr(&tknzer->tmp_wrd);
    tknzer->tmp_wrd.arr[tknzer->tmp_wrd.idx] = input->str[input->cur_idx];
    (tknzer->tmp_wrd.idx)++;
}

static void process_end_of_word(type_tokenizer *tknzer)
{
    tknzer->tmp_wrd.arr[tknzer->tmp_wrd.idx] = '\0';
    tknzer->words_list.last->word =
        malloc((tknzer->tmp_wrd.idx + 1) * sizeof(char));
    strcpy(tknzer->words_list.last->word, tknzer->tmp_wrd.arr);
    tknzer->tmp_wrd.idx = 0;
}

static void complete_word(type_tokenizer *tknzer)
{
    if (!tknzer->word_ended && !words_list_is_empty(tknzer)) {
        process_end_of_word(tknzer);
        tknzer->word_ended = true;
    }
}

static void process_space_character(type_tokenizer *tknzer, type_input *input)
{
    if (tknzer->quotation)
        add_character_to_word(tknzer, input);
    else
        complete_word(tknzer);
}

static void process_escaped_character(type_tokenizer *tknzer, type_input *input)
{
    add_character_to_word(tknzer, input);
    tknzer->char_escaping = false;
}

static void process_escape_character(type_tokenizer *tknzer, type_input *input)
{
    if (tknzer->char_escaping) {
        process_escaped_character(tknzer, input);
    } else
        tknzer->char_escaping = true;
}

static void possible_case_of_adding_empty_word(
    type_tokenizer *tknzer, type_input *input
);

static void process_quotation_mark_character(
    type_tokenizer *tknzer, type_input *input
)
{
    if (tknzer->char_escaping) {
        process_escaped_character(tknzer, input);
    } else {
        toggle_quotation(tknzer);
        possible_case_of_adding_empty_word(tknzer, input);
    }
}

static separator_type get_separator_val(char c)
{
    switch (c) {
        case ('&'):
            return background_operator;
        case ('>'):
            return output_redirection;
        case ('|'):
            return pipe_operator;
        case ('<'):
            return input_redirection;
        case (';'):
            return command_separator;
        case ('('):
            return open_parenthesis;
        case (')'):
            return close_parenthesis;
        default:
            fprintf(stderr, "%s, %d: Something went wrong\n", __FILE__, __LINE__);
            return -1;
    }
}

static separator_type get_double_separator_val(char c)
{
    switch (c) {
        case ('&'):
            return and_operator;
        case ('>'):
            return output_append_redirection;
        case ('|'):
            return or_operator;
        default:
            fprintf(stderr, "%s, %d: Something went wrong\n", __FILE__, __LINE__);
            return -1;
    }
}

static void add_separator(type_tokenizer *tknzer, separator_type separator)
{
    tknzer->words_list.last->separator_val = separator;
    complete_word(tknzer);
}

void process_character(type_tokenizer *tknzer);

static void process_separator(type_tokenizer *tknzer, type_input *input)
{
    if (tknzer->quotation)
        add_character_to_word(tknzer, input);
    else {
        /* complete the previous word */
        complete_word(tknzer);
        add_character_to_word(tknzer, input);
        add_separator(tknzer, get_separator_val(input->str[input->cur_idx]));
    }
}

static void process_possible_double_separator(
    type_tokenizer *tknzer, type_input *input
)
{
    if (tknzer->quotation)
        add_character_to_word(tknzer, input);
    else {
        char chr = input->str[input->cur_idx];
        /* complete the previous word */
        complete_word(tknzer);
        add_character_to_word(tknzer, input);
        input->cur_idx++;
        if (input->str[input->cur_idx] == chr) {
            add_character_to_word(tknzer, input);
            add_separator(
                tknzer, get_double_separator_val(input->str[input->cur_idx])
            );
        } else {
            add_separator(tknzer, get_separator_val(chr));
            parse_next_character(tknzer, input);
        }
    }
}

static bool incorrect_character_escaping(
    const type_tokenizer *tknzer, const type_input *input
)
{
    if (
        (tknzer->char_escaping) &&
        (input->str[input->cur_idx] != '\\') &&
        (input->str[input->cur_idx] != '"')
    )
        return true;
    else
        return false;
}

static void handle_incorrect_character_escaping(type_tokenizer *tknzer)
{
    tknzer->err_code = incorrect_char_escaping;
    tknzer->str_ended = true;
}
void parse_next_character(type_tokenizer *tknzer, type_input *input)
{
    if (incorrect_character_escaping(tknzer, input)) {
        handle_incorrect_character_escaping(tknzer);
        return;
    }
    switch (input->str[input->cur_idx]) {
        case ('\t'):
        case (' '):
            process_space_character(tknzer, input);
            break;
        case ('\n'):
            complete_word(tknzer);
            tknzer->str_ended = true;
            break;
        case ('\\'):
            process_escape_character(tknzer, input);
            break;
        case ('"'):
            process_quotation_mark_character(tknzer, input);
            break;
        case ('<'):
        case (';'):
        case ('('):
        case (')'):
            process_separator(tknzer, input);
            break;
        case ('&'):
        case ('>'):
        case ('|'):
            process_possible_double_separator(tknzer, input);
            break;
        default:
            add_character_to_word(tknzer, input);
    }
}

static bool space_character(char c)
{
    return (c == ' ' || c == '\t' || c == '\n');
}

static void possible_case_of_adding_empty_word(
    type_tokenizer *tknzer, type_input *input
)
{
    if ((!tknzer->words_list.last) || (tknzer->word_ended)) {
        input->cur_idx++;
        if (input->str[input->cur_idx] == '"') {
            toggle_quotation(tknzer);
            input->cur_idx++;
            if (space_character(input->str[input->cur_idx]))
                add_empty_item_to_list_of_words(&tknzer->words_list);
        }
        parse_next_character(tknzer, input);
    }
}
