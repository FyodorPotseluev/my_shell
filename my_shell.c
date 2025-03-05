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

extern char *environ;

enum consts {
    init_tmp_wrd_arr_len    = 16,
    init_cmd_line_arr_len   = 16
};

#define ESC_ERR "my_shell: Error: only the characters `\"` and `\\` can be escaped\n"

typedef enum tag_error_code {
    no_error,
    incorrect_char_escaping,
    background_operator_not_in_the_end_of_str,
    not_implemented_feature
} error_code;

typedef enum tag_separator_type {
    none,
    background_operator,
    and_operator,
    output_redirection,
    output_append_redirection,
    pipe_operator,
    or_operator,
    input_redirection,
    command_separator,
    open_parenthesis,
    close_parenthesis
} separator_type;

typedef struct tag_word_item {
    char *word;
    separator_type separator_val;
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
    bool word_ended, str_ended, quotation, char_escaping;
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

bool there_are_possible_zombies_left(int res)
{
    /* if we get a valid pid */
    return ((res != 0) && (res != -1)) ? true : false;
}

void cleanup_background_zombies()
{
    int res;
    do {
        res = wait4(-1, NULL, WNOHANG, NULL);
    } while (there_are_possible_zombies_left(res));
}

#if defined(PRINT_TOKENS_MODE)
void print_separator_value(separator_type separator_val)
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

void print_list_of_words(const curr_str_words_list *words_list)
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
#elif defined(EXEC_MODE)
void increase_cmd_line_array_length(string *str)
{
    free(str->cmd_line.arr);
    while (str->cmd_line.arr_len < str->words_list.len)
        str->cmd_line.arr_len *= 2;
    str->cmd_line.arr = malloc(str->cmd_line.arr_len * sizeof(char*));
}

error_code transform_words_list_into_cmd_line_arr(
    string *str, bool *background_execution
)
{
    word_item *p = str->words_list.first;
    int i = 0;
    if (str->cmd_line.arr_len < str->words_list.len)
        increase_cmd_line_array_length(str);
    for (;; p=p->next, i++) {
        if (!p) {
            str->cmd_line.arr[i] = NULL;
            break;
        } else {
            /* handle possible separator */
            if (p->separator_val == background_operator) {
                str->cmd_line.arr[i] = NULL;
                *background_execution = true;
                continue;
            } else
            if (*background_execution)
                return background_operator_not_in_the_end_of_str;
            else
            if (p->separator_val != none)
                return not_implemented_feature;
            /* ------------------------- */
            str->cmd_line.arr[i] = p->word;
        }
    }
    return 0;
}

void change_to_home_directory()
{
    int res;
    const char *home_dir = getenv("HOME");
    if (!home_dir) {
        fprintf(stderr, "%s, %d, %s: ", __FILE__, __LINE__, "getenv");
        perror("");
        return;
    }
    res = chdir(home_dir);
    error_handling(res, __FILE__, __LINE__, "chdir");
}

void handle_change_dir_command(const string *str)
{
    if (!str->cmd_line.arr[1])
        change_to_home_directory();
    else
    if (!str->cmd_line.arr[2]) {
        int res = chdir(str->cmd_line.arr[1]);
        error_handling(res, __FILE__, __LINE__, "chdir");
    } else
        fprintf(stderr, "my_shell: cd: too many arguments\n");
}

bool change_dir_command(const string *str)
{
    return (0 == strcmp("cd", str->cmd_line.arr[0])) ? true : false;
}

void print_error(error_code err)
{
    switch (err) {
        case (no_error):
            fprintf(stderr, "%s, %d: Something went wrong\n", __FILE__, __LINE__);
            break;
        case (incorrect_char_escaping):
            fprintf(stderr, "%s, %d: Something went wrong\n", __FILE__, __LINE__);
            break;
        case (background_operator_not_in_the_end_of_str):
            fprintf(
                stderr,
                "my_shell: Error: background operator not in the end of string\n"
            );
            break;
        case (not_implemented_feature):
            fprintf(stderr, "my_shell: feature not implemented yet\n");
    }
}

void handle_zombies(bool background_execution, int pid)
{
    int res;
    if (background_execution)
        cleanup_background_zombies();
    else {
        /* cleanup the single foreground zombie */
        do {
            res = wait(NULL);
            error_handling(res, __FILE__, __LINE__, "wait");
        } while (res != pid);
    }
}

int launch_process(string *str)
{
    int pid;
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
    return pid;
}
#endif

bool words_list_is_empty(string *str)
{
    return (!str->words_list.first) ? true : false;
}

void execute_command(string *str)
{
#if defined(PRINT_TOKENS_MODE)
    print_list_of_words(&str->words_list);
#elif defined(EXEC_MODE)
    bool background_execution = false;
    int pid;
    error_code err = 0;
    if (words_list_is_empty(str))
        return;
    err = transform_words_list_into_cmd_line_arr(str, &background_execution);
    if (err) {
        print_error(err);
        return;
    }
    if (change_dir_command(str)) {
        handle_change_dir_command(str);
        return;
    }
    pid = launch_process(str);
    handle_zombies(background_execution, pid);
#endif
}

void toggle_quotation(string *str)
{
    if (str->quotation)
        str->quotation = false;
    else
        str->quotation = true;
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
    list->last->separator_val = none;
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
    if (str->tmp_wrd.idx == 0)
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
    str->quotation = false;
    str->char_escaping = false;
    str->err_code = no_error;
    str->words_list.len = 1;
    str->tmp_wrd.idx = 0;
}

bool report_if_error(const string *str)
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

void stdin_cleanup()
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

void complete_word(string *str)
{
    if (!str->word_ended && !words_list_is_empty(str)) {
        process_end_of_word(str);
        str->word_ended = true;
    }
}

void process_space_character(string *str)
{
    if (str->quotation)
        add_character_to_word(str);
    else
        complete_word(str);
}

void process_escaped_character(string *str)
{
    add_character_to_word(str);
    str->char_escaping = false;
}

void process_escape_character(string *str)
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
        toggle_quotation(str);
        possible_case_of_adding_empty_word(str);
    }
}

separator_type get_separator_val(char c)
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

separator_type get_double_separator_val(char c)
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

void add_separator(string *str, separator_type separator)
{
    str->words_list.last->separator_val = separator;
    complete_word(str);
}

void process_character(string *str);

void process_separator(string *str)
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

void process_possible_double_separator(string *str)
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

bool incorrect_character_escaping(string *str)
{
    if ((str->char_escaping) && (str->c != '\\') && (str->c != '"'))
        return true;
    else
        return false;
}

void handle_incorrect_character_escaping(string *str)
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

void possible_case_of_adding_empty_word(string *str)
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
