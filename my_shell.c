/* my_shell.c */

#if !defined(EXEC_MODE) && !defined(PRINT_TOKENS_MODE)
#error Please define either EXEC_MODE or PRINT_TOKENS_MODE
#endif

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

enum consts {
    init_tmp_wrd_arr_len    = 16,
    init_cmd_line_arr_len   = 16
};

typedef enum tag_error_code {
    no_error, incorrect_char_escaping
} error_code;

typedef struct tag_word_item {
    char *word;
    struct tag_word_item *next;
} word_item;

typedef struct tag_curr_str_words_list {
    word_item *first;
    word_item *last;
    int len;
} curr_str_words_list;

typedef struct tag_curr_word_dynamic_char_arr {
    char *arr;
    int idx;
    int arr_len;
} curr_word_dynamic_char_arr;

typedef struct tag_execvp_cmd_line {
    char **arr;
    int arr_len;
} execvp_cmd_line;

typedef struct tag_string {
    bool word_ended, str_ended, include_spaces, char_escaping;
    int c;
    error_code err_code;
    /* contains the array in which the current word is formed */
    curr_word_dynamic_char_arr tmp_wrd;
    /* contains the linked list in wich the current string is stored */
    curr_str_words_list words_list;
    /* contains the array designated for the `execvp` call */
    execvp_cmd_line cmd_line;
} string;

void error_handling(
    int res, const char *file_name, int line_num, const char *cmd
)
{
    if (res == -1) {
        fprintf(stderr, "%s, %d, %s: ", file_name, line_num, cmd);
        perror("");
        fflush(stderr);
    }
}

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

#if defined(PRINT_TOKENS_MODE)
void print_list_of_words(const curr_str_words_list *words_list)
{
    word_item *p = words_list->first;
    while (p) {
        printf("[%s]\n", p->word ? p->word : "");
        p = p->next;
    }
}
#elif defined(EXEC_MODE)
void increase_cmd_line_array_length(string *str)
{
    free(str->cmd_line.arr);
    while (str->cmd_line.arr_len < str->words_list.len)
        str->cmd_line.arr_len *= 2;
    str->cmd_line.arr = malloc(str->cmd_line.arr_len * sizeof(char*));
}

void transform_words_list_into_cmd_line_arr(string *str)
{
    word_item *p = str->words_list.first;
    int i = 0;
    if (str->cmd_line.arr_len < str->words_list.len)
        increase_cmd_line_array_length(str);
    for (;; p=p->next, i++) {
        if (!p) {
            str->cmd_line.arr[i] = NULL;
            break;
        }
        str->cmd_line.arr[i] = p->word;
    }
}

void handle_change_dir_command(const string *str)
{
    if (!str->cmd_line.arr[1])
        printf("Change dir to HOME. Will implement the feature later\n");
    else
    if (!str->cmd_line.arr[2]) {
        int res = chdir(str->cmd_line.arr[1]);
        error_handling(res, __FILE__, __LINE__, "chdir");
    } else
        printf("my_shell: cd: too many arguments\n");
}

bool change_dir_command(const string *str)
{
    return (0 == strcmp("cd", str->cmd_line.arr[0])) ? true : false;
}
#endif

void execute_command(string *str)
{
#if defined(PRINT_TOKENS_MODE)
    print_list_of_words(&str->words_list);
#elif defined(EXEC_MODE)
    int pid, res;
    if (!str->words_list.first)
        return;
    transform_words_list_into_cmd_line_arr(str);
    if (change_dir_command(str)) {
        handle_change_dir_command(str);
        return;
    }
    fflush(stderr);
    pid = fork();
    error_handling(pid, __FILE__, __LINE__, "fork");
    if (pid == 0) {
        execvp(str->cmd_line.arr[0], str->cmd_line.arr);
        fprintf(stderr, "%s, %d, %s: ", __FILE__, __LINE__, "execvp");
        perror("");
        fflush(stderr);
        _exit(1);
    }
    res = wait(NULL);
    error_handling(res, __FILE__, __LINE__, "wait");
#endif
}

void toggle_include_spaces(string *str)
{
    if (str->include_spaces)
        str->include_spaces = false;
    else
        str->include_spaces = true;
}

void add_empty_item_to_list_of_words(curr_str_words_list *list)
{
    if (!list->first) {
        list->first = malloc(sizeof(word_item));
        list->last = list->first;
    } else {
        list->last->next = malloc(sizeof(word_item));
        list->last = list->last->next;
    }
    list->last->word = NULL;
    list->last->next = NULL;
    list->len += 1;
}

void double_tmp_wrd_arr(curr_word_dynamic_char_arr *tmp_wrd)
{
    char *doubled_arr = malloc(tmp_wrd->arr_len*2 * sizeof(char));
    memcpy(doubled_arr, tmp_wrd->arr, tmp_wrd->arr_len);
    free(tmp_wrd->arr);
    tmp_wrd->arr = doubled_arr;
    tmp_wrd->arr_len *= 2;
}

void add_character_to_word(string *str)
{
    str->word_ended = false;
    /* if we haven't started to write the `tmp_wrd` yet */
    if (!str->tmp_wrd.idx)
        add_empty_item_to_list_of_words(&str->words_list);
    if (str->tmp_wrd.idx == str->tmp_wrd.arr_len-1)
        double_tmp_wrd_arr(&str->tmp_wrd);
    str->tmp_wrd.arr[str->tmp_wrd.idx] = str->c;
    (str->tmp_wrd.idx)++;
}

void process_end_of_word(string *str)
{
    str->tmp_wrd.arr[str->tmp_wrd.idx] = '\0';
    str->words_list.last->word = malloc((str->tmp_wrd.idx + 1) * sizeof(char));
    strcpy(str->words_list.last->word, str->tmp_wrd.arr);
    str->tmp_wrd.idx = 0;
}

void reset_str_variables(string *str)
{
    str->word_ended = false;
    str->str_ended = false;
    str->include_spaces = false;
    str->char_escaping = false;
    str->err_code = no_error;
    str->words_list.len = 1;
    str->tmp_wrd.idx = 0;
}

bool report_if_error(const string *str)
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
    if (str->err_code == no_error)
        error = false;
    return error;
}

void process_end_of_string(string *str)
{
    bool error = report_if_error(str);
    if (!error)
        execute_command(str);
    free_list_of_words(&str->words_list);
    reset_str_variables(str);
    printf("> ");
}

void complete_word(string *str)
{
    if (!str->word_ended && str->words_list.last) {
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
    if ((!str->words_list.last) || (str->word_ended)) {
        str->c = getchar();
        if (str->c == '"') {
            toggle_include_spaces(str);
            str->c = getchar();
            if (str->c == ' ' || str->c == '\t' || str->c == '\n')
                add_empty_item_to_list_of_words(&str->words_list);
        }
        process_character(str);
    }
}

int main()
{
    string str = {
        false, false, false, false, 0, no_error,
        { NULL, 0, init_tmp_wrd_arr_len },
        { NULL, NULL, 1 },
        { NULL, init_cmd_line_arr_len }
    };
    str.tmp_wrd.arr = malloc(str.tmp_wrd.arr_len * sizeof(char));
    str.cmd_line.arr = malloc(str.cmd_line.arr_len * sizeof(char*));
    printf("> ");
    while ((str.c = getchar()) != EOF) {
        process_character(&str);
        if (str.str_ended)
            process_end_of_string(&str);
    }
    free_list_of_words(&str.words_list);
    free(str.tmp_wrd.arr);
    str.tmp_wrd.arr = NULL;
    free(str.cmd_line.arr);
    printf("^D\n");
    return 0;
}
