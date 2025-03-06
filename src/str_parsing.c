/* str_parsing.c */

#include "cmd_execution.h"
#include "str_parsing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ESC_ERR "my_shell: Error: only the characters `\"` and `\\` can be escaped\n"

void free_list_of_words(curr_str_words_list *link_list)
{
    word_item *p = link_list->first;
    while (p) {
        word_item *tmp = p;
        p = p->next;
        free(tmp->word);
        free(tmp);
    }
    link_list->first = NULL;
    link_list->last = NULL;
    link_list->len = 1;
}

static void toggle_quotation(string *str)
{
    if (str->quotation)
        str->quotation = false;
    else
        str->quotation = true;
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
    list->len += 1;
}

static void double_tmp_wrd_arr(curr_word_dynamic_char_arr *tmp_wrd)
{
    char *doubled_arr = malloc(tmp_wrd->arr_len*2 * sizeof(char));
    memcpy(doubled_arr, tmp_wrd->arr, tmp_wrd->arr_len);
    free(tmp_wrd->arr);
    tmp_wrd->arr = doubled_arr;
    tmp_wrd->arr_len *= 2;
}

static void add_character_to_word(string *str)
{
    str->word_ended = false;
    /* if we haven't started to write the `tmp_wrd` yet */
    if (str->tmp_wrd.idx == 0)
        add_empty_item_to_list_of_words(&str->words_list);
    if (str->tmp_wrd.idx == str->tmp_wrd.arr_len-1)
        double_tmp_wrd_arr(&str->tmp_wrd);
    str->tmp_wrd.arr[str->tmp_wrd.idx] = str->c;
    (str->tmp_wrd.idx)++;
}

static void process_end_of_word(string *str)
{
    str->tmp_wrd.arr[str->tmp_wrd.idx] = '\0';
    str->words_list.last->word = malloc((str->tmp_wrd.idx + 1) * sizeof(char));
    strcpy(str->words_list.last->word, str->tmp_wrd.arr);
    str->tmp_wrd.idx = 0;
}

static void reset_str_variables(string *str)
{
    str->word_ended = false;
    str->str_ended = false;
    str->quotation = false;
    str->char_escaping = false;
    str->err_code = no_error;
    str->words_list.len = 1;
    str->tmp_wrd.idx = 0;
}

static bool report_if_error(const string *str)
{
    bool error = true;
    if (str->err_code == incorrect_char_escaping)
        fprintf(stderr, "%s", ESC_ERR);
    else
    /* check this error condition before last */
    /* the `stdin_cleanup` function, which could be called if previous errors
    were detected, could break the balance of quote signs */
    if (str->quotation)
        fprintf(stderr, "my_shell: Error: unmatched quotes\n");
    else
    /* check this error condition last */
    if (str->err_code == no_error)
        error = false;
    return error;
}

static void stdin_cleanup()
{
    int c;
    while ((c=getchar() != '\n') && (c != EOF))
        ;
}

void process_end_of_string(string *str)
{
    bool error = report_if_error(str);
    if (!error)
        execute_command(str);
    free_list_of_words(&str->words_list);
    reset_str_variables(str);
    cleanup_background_zombies();
    printf("> ");
}

static void complete_word(string *str)
{
    if (!str->word_ended && !words_list_is_empty(str)) {
        process_end_of_word(str);
        str->word_ended = true;
    }
}

static void process_space_character(string *str)
{
    if (str->quotation)
        add_character_to_word(str);
    else
        complete_word(str);
}

static void process_escaped_character(string *str)
{
    add_character_to_word(str);
    str->char_escaping = false;
}

static void process_escape_character(string *str)
{
    if (str->char_escaping) {
        process_escaped_character(str);
    } else
        str->char_escaping = true;
}

static void possible_case_of_adding_empty_word(string *str);

static void process_quotation_mark_character(string *str)
{
    if (str->char_escaping) {
        process_escaped_character(str);
    } else {
        toggle_quotation(str);
        possible_case_of_adding_empty_word(str);
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

static void add_separator(string *str, separator_type separator)
{
    str->words_list.last->separator_val = separator;
    complete_word(str);
}

void process_character(string *str);

static void process_separator(string *str)
{
    if (str->quotation)
        add_character_to_word(str);
    else {
        /* complete the previous word */
        complete_word(str);
        add_character_to_word(str);
        add_separator(str, get_separator_val(str->c));
    }
}

static void process_possible_double_separator(string *str)
{
    if (str->quotation)
        add_character_to_word(str);
    else {
        char chr = str->c;
        /* complete the previous word */
        complete_word(str);
        add_character_to_word(str);
        str->c = getchar();
        if (str->c == chr) {
            add_character_to_word(str);
            add_separator(str, get_double_separator_val(str->c));
        } else {
            add_separator(str, get_separator_val(chr));
            process_character(str);
        }
    }
}

static bool incorrect_character_escaping(const string *str)
{
    if ((str->char_escaping) && (str->c != '\\') && (str->c != '"'))
        return true;
    else
        return false;
}

static void handle_incorrect_character_escaping(string *str)
{
    str->err_code = incorrect_char_escaping;
    str->str_ended = true;
    stdin_cleanup();
}

void process_character(string *str)
{
    if (incorrect_character_escaping(str)) {
        handle_incorrect_character_escaping(str);
        return;
    }
    switch (str->c) {
        case ('\t'):
        case (' '):
            process_space_character(str);
            break;
        case ('\n'):
            complete_word(str);
            str->str_ended = true;
            break;
        case ('\\'):
            process_escape_character(str);
            break;
        case ('"'):
            process_quotation_mark_character(str);
            break;
        case ('<'):
        case (';'):
        case ('('):
        case (')'):
            process_separator(str);
            break;
        case ('&'):
        case ('>'):
        case ('|'):
            process_possible_double_separator(str);
            break;
        default:
            add_character_to_word(str);
    }
}

static void possible_case_of_adding_empty_word(string *str)
{
    if ((!str->words_list.last) || (str->word_ended)) {
        str->c = getchar();
        if (str->c == '"') {
            toggle_quotation(str);
            str->c = getchar();
            if (str->c == ' ' || str->c == '\t' || str->c == '\n')
                add_empty_item_to_list_of_words(&str->words_list);
        }
        process_character(str);
    }
}
