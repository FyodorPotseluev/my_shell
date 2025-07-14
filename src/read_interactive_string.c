/* read_interactive_string.c */

#include "read_interactive_string.h"
#include "handle_signals.h"
#include "handle_err.h"
#include "shut_down_my_shell.h"
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#if defined(LOG)
#  include <unistd.h>

extern int log_fd;

#  define TAB           "(Tab)"
#  define BACKSPACE     "(Bksp)"
#  define ENTER         "(Enter)\n"
#  define DELETE        "(Del)"
#  define LEFT_ARROW    "(<-)"
#  define RIGHT_ARROW   "(->)"
#  define CTRL_W        "(Ctrl + W)"
#  define CTRL_U        "(Ctrl + U)"
#  define CTRL_D        "(Ctrl + D)\n"

#endif

typedef enum tag_type_direction { left, right } type_direction;

enum key_code {
    esc                 = 27,
    backspace           = 127,
    ctrl_d              = 4,
    ctrl_u              = 21,
    ctrl_w              = 23,
    delete              = 27+91+51+126
};

static bool before_1st_wrd(const type_input *input)
{
    return (input->cur_idx <= input->_1st_wrd_start_idx);
}

static bool delete_before_1st_wrd(const type_input *input)
{
    return (input->cur_idx < input->_1st_wrd_start_idx);
}

static bool in_1st_wrd(const type_input *input)
{
    if (input->cur_idx > input->_1st_wrd_start_idx &&
        input->cur_idx <= input->_1st_wrd_end_idx)
    {
        return true;
    } else
        return false;
}

static bool inside_1st_wrd(const type_input *input)
{
    if (input->cur_idx >= input->_1st_wrd_start_idx &&
        input->cur_idx < input->_1st_wrd_end_idx)
    {
        return true;
    } else
        return false;
}

static bool cur_idx_inside_1st_wrd(const type_input *input)
{
    if ((input->cur_idx >= input->_1st_wrd_start_idx) &&
        (input->cur_idx <= input->_1st_wrd_end_idx))
    {
        return true;
    } else {
        return false;
    }
}

static bool right_after_1st_wrd(const type_input *input, int idx)
{
    return (input->cur_idx == input->_1st_wrd_end_idx+idx);
}

static bool ctrl_w_after_1st_wrd_but_before_2nd_wrd(
    const type_input *input, int removed_len
)
{
    if (input->cur_idx == input->_1st_wrd_end_idx ||
        (input->cur_idx > input->_1st_wrd_end_idx &&
            input->cur_idx - removed_len == input->_1st_wrd_start_idx))
    {
        return true;
    } else
        return false;
}

static bool ctrl_u_after_1st_wrd(const type_input *input)
{
    return (input->cur_idx >= input->_1st_wrd_end_idx);
}

static int find_new_1st_wrd_end_idx(type_input *input, int idx)
{
    int i = idx;
    if (input->cur_idx + idx == input->end_idx)
        return input->_1st_wrd_end_idx;
    while (true) {
        if (input->str[input->cur_idx + i] == ' ' ||
            input->str[input->cur_idx + i] == '\0')
        {
            return input->cur_idx + i - 1;
        }
        else
            i++;
    }
}

static void find_new_1st_wrd_indices(
    type_input *input, int start_pos)
{
    int i = start_pos;
    while (true) {
        if (input->cur_idx + i >= input->end_idx) {
            input->_1st_wrd_start_idx = -1;
            input->_1st_wrd_end_idx = -1;
            return;
        }
        if (input->str[input->cur_idx + i] != ' ') {
            input->_1st_wrd_start_idx = input->cur_idx + i;
            break;
        }
        i++;
    }
    while (true) {
        if (input->cur_idx + i == input->end_idx ||
            input->str[input->cur_idx + i] == ' ')
        {
            input->_1st_wrd_end_idx = input->cur_idx + i;
            break;
        }
        i++;
    }
}

static bool there_is_no_slash_in_1st_wrd(const type_input *input)
{
    int i = input->_1st_wrd_start_idx;
    for (; i < input->_1st_wrd_end_idx; i++)
        if (input->str[i] == '/')
            return false;
    return true;
}

void del_chr_handle_possible_1st_wrd_termination(
    enum key_code del_chr_type, type_input *input
)
{
    int start_pos;
    if (del_chr_type == backspace)
        start_pos = 0;
    else
    if (del_chr_type == delete)
        start_pos = 1;
    if (input->_1st_wrd_end_idx == input->_1st_wrd_start_idx)
        find_new_1st_wrd_indices(input, start_pos);
}

static void del_chr_handle_1st_wrd(
    enum key_code del_chr_type, type_input *input
)
{
    if ((del_chr_type == backspace && /* backspace */before_1st_wrd(input)) ||
        (del_chr_type == delete && delete_before_1st_wrd(input)))
    {
        input->_1st_wrd_start_idx--;
        input->_1st_wrd_end_idx--;
    }
    else
    if ((del_chr_type == backspace && /* backspace */in_1st_wrd(input)) ||
        (del_chr_type == delete && /* delete */inside_1st_wrd(input)))
    {
        input->_1st_wrd_end_idx--;
        del_chr_handle_possible_1st_wrd_termination(del_chr_type, input);
    }
    else
    if (del_chr_type == backspace && /*backspace*/right_after_1st_wrd(input, 1))
        input->_1st_wrd_end_idx = find_new_1st_wrd_end_idx(input, 0);
    else
    if (del_chr_type == delete && /* delete */right_after_1st_wrd(input, 0))
        input->_1st_wrd_end_idx = find_new_1st_wrd_end_idx(input, 1);
}

static bool old_1st_wrd_no_longer_exists_and_there_isnt_a_new_one(
    const type_input *input
)
{
    return (input->_1st_wrd_start_idx == -1) || (input->_1st_wrd_end_idx == -1);
}

static void del_chr_sequence_handle_1st_wrd(
    enum key_code del_chr_sequence_type,
    type_input *input, int removed_len
)
{
    if (/* chr_sequence_deletion */before_1st_wrd(input))
        input->_1st_wrd_start_idx -= removed_len;
    else
    if (del_chr_sequence_type == ctrl_u && /* ctrl_u */inside_1st_wrd(input))
        input->_1st_wrd_start_idx = 0;
    if ((del_chr_sequence_type == ctrl_w &&
        ctrl_w_after_1st_wrd_but_before_2nd_wrd(input, removed_len))
        ||
        (del_chr_sequence_type == ctrl_u &&
        ctrl_u_after_1st_wrd(input)))
    {
        find_new_1st_wrd_indices(input, 0);
        if (old_1st_wrd_no_longer_exists_and_there_isnt_a_new_one(input))
            return;
        input->_1st_wrd_start_idx -= removed_len;
    }
    input->_1st_wrd_end_idx -= removed_len;
}

static bool its_a_new_wrd_before_old_1st_wrd(const type_input *input)
{
    return (input->cur_idx < input->_1st_wrd_start_idx);
}

static bool its_the_very_1st_input_wrd(const type_input *input)
{
    return (input->_1st_wrd_start_idx == -1);
}

static void default_char_handle_1st_wrd(type_input *input)
{
    if (its_a_new_wrd_before_old_1st_wrd(input) ||
        its_the_very_1st_input_wrd(input))
    {
        input->_1st_wrd_start_idx = input->cur_idx;
        input->_1st_wrd_end_idx = input->cur_idx + 1;
    }
    else
    if (cur_idx_inside_1st_wrd(input)) {
        input->_1st_wrd_end_idx++;
    }
}

static void return_cursor_back_n_positions(int n)
{
    int i;
    for (i=0; i < n; i++) {
        putchar('\b');
    }
}

static void reprint_the_shift_of_the_string_to_the_(
    type_direction direction, const type_input *input
)
{
    if (direction == left)
        /*
        Backspace in the middle of a word:

                    1234rest_of_str
        curr_idx:       ^
                    123 rest_of_str
        curr_idx:      ^
                    ```
                    reprint_the_shift_of_the_string_to_the_(left, input);
                    ```
                    123rest_of_strr
        curr_idx:      ^          ^
                   _______________|  extra character to cover with extra space
                  |
                  v                                                         */
        printf("%s \b", input->str + input->cur_idx + 1);
    else
    if (direction == right)
        /*
        Put character in the middle of a word:

                    abcdrest_of_str
        curr_idx:       ^
                    abcdXest_of_str
        curr_idx:       ^
                    ```
                    reprint_the_shift_of_the_string_to_the_(right, input);
                    ```
                    abcdXrest_of_str
        curr_idx:       ^
        */
        printf("%s", input->str + input->cur_idx);
}

static void move_string_arr_to_the_(
    type_direction direction, const type_input *input
)
{
    char *left_side = input->str + input->cur_idx;
    char *right_side = input->str + input->cur_idx + 1;
    if (direction == left)
        /*
                    str1 str2
        curr_idx:       ^
                    str1str2
        curr_idx:       ^
        */
        memmove(left_side, right_side, input->end_idx - input->cur_idx);
    else
    if (direction == right)
        /*
                    str1str2
        curr_idx:       ^
                    str1sstr2
        curr_idx:       ^
                                                                zero byte  */
        memmove(right_side, left_side, input->end_idx - input->cur_idx + 1);
}

static void add_character_to_str_arr(char c, int idx, type_input *input)
{
    /* we also need space in case the string array will be moved to the right */
    if (input->end_idx + 1 >= input->str_len) {
        /* increase string */
        input->str_len *= 2;
        input->str = realloc(input->str, input->str_len*sizeof(char));
    }
    if (idx != input->end_idx)
        move_string_arr_to_the_(right, input);
    input->str[idx] = c;
}

static void reprint_current_input(const type_input *input)
{
    printf("> %s", input->str);
    return_cursor_back_n_positions(input->end_idx - input->cur_idx);
}

char *get_dir_name_from_curr_wrd(char *curr_wrd, int *slash_idx)
{
    int i = 0;
    while (curr_wrd[i] != '\0') {
        if (curr_wrd[i] == '/')
            *slash_idx = i;
        i++;
    }
    if (*slash_idx != -1) {
        curr_wrd[*slash_idx] = '\0';
        return curr_wrd;
    } else
        /* there were no slash in the `curr_wrd` */
        return NULL;
}

static bool invalid_dir_name()
{
    return (errno = ENOENT);
}

static void process_the_start_of_the_str(
    char *dir_name, int *i, int *dir_name_end_idx,
    const char **curr_wrd, type_path *path
)
{
    if (**curr_wrd == '/') {
        /* it's the root directory */
        dir_name[*i] = '/';
        *i = *dir_name_end_idx = path->end_idx = *i + 1;
        (*curr_wrd)++;
    } else
    if (**curr_wrd == '.') {
        if (*(*curr_wrd + 1) == '/') {
            /* it's the current directory specified explicitly */
            path->end_idx = sizeof("./") - 1;
            *curr_wrd += sizeof("./") - 1;
        }
        goto add_curr_dir_path;
    } else {
        /* it's the current directory specified implicitly*/
        add_curr_dir_path:
        strcpy(dir_name, "./");
        *i = *dir_name_end_idx = sizeof("./") - 1;
    }
}

static void get_dir_name_and_search_file_name_from_curr_wrd(
    char *dir_name,  int *dir_name_end_idx, const char **search_file_name,
    const char *curr_wrd, type_path *path

)
{
    int i = 0, shift;
    const char *init_curr_wrd = curr_wrd;
    process_the_start_of_the_str(
        dir_name, &i, dir_name_end_idx, &curr_wrd, path
    );
    *search_file_name = curr_wrd;
    shift = *dir_name_end_idx - (curr_wrd - init_curr_wrd);
    for (; *curr_wrd; i++, curr_wrd++) {
                             /* if curr chr is '/' we need one extra space */
        if (i > dir_name_buf_size - 2) {
            fprintf(stderr, ERR_DIR_NAME_BUFFER_OVERFLOW, __FILE__, __LINE__);
            dir_name[i] = '\0';
            return;
        }
        if (*curr_wrd == '/') {
            *dir_name_end_idx = i + 1;
            path->end_idx = *dir_name_end_idx - shift;
            *search_file_name = curr_wrd + 1;
        }
        dir_name[i] = *curr_wrd;
    }
    dir_name[*dir_name_end_idx] = '\0';
}

static bool prepare_args_for_filesystem_autocompletion(
    const char *curr_wrd, const char **search_file_name,
    type_tst **dir_files_tree, type_path *path
)
{
    DIR *dir = NULL;
    int res, dir_name_end_idx;
    char dir_name[dir_name_buf_size];
    get_dir_name_and_search_file_name_from_curr_wrd(
        dir_name, &dir_name_end_idx, search_file_name, curr_wrd, path
    );
    dir = opendir(dir_name);
    if (!dir) {
        if (invalid_dir_name())
            return false;
        else
            pointer_error_handling(dir, __FILE__, __LINE__,"opendir");
    }
    add_dir_files_names_to_tst(dir, dir_files_tree, dir_name, dir_name_end_idx);
    res = closedir(dir);
    error_handling(res, __FILE__, __LINE__, "closedir");
    return true;
}

static autocomplete_matches_found
filesystem_autocompletion_or_printing_word_options(
    char *match_str, const char *curr_wrd, int num_of_tab_key_presses
)
{
    autocomplete_matches_found res;
    const char *search_file_name = NULL;
    type_tst *dir_files_tree = NULL;
    type_path path = { curr_wrd, -1 };
    bool dir_name_is_valid = prepare_args_for_filesystem_autocompletion(
        curr_wrd, &search_file_name, &dir_files_tree, &path
    );
    if (!dir_name_is_valid)
        return no_matches;
    /* mutable reference of `match_str` to `ternary_search_tree` module */
    /* immutable reference of `input` (`curr_wrd`, `search_file_name` and
    `path` are pointers to it) to `ternary_search_tree` module */
    /* immutable reference of `dir_files_tree` to `ternary_search_tree` module*/
    res = tst_find_auto_completion_or_print_word_options(
        match_str, search_file_name, dir_files_tree,
        num_of_tab_key_presses, &path
    );
    free_tst(dir_files_tree);
    return res;
}

static autocomplete_matches_found find_auto_completion_or_print_word_options(
    char *match_str, const type_input *input, const type_tst *path_tree
)
{
    autocomplete_matches_found res = no_matches;
    char *curr_wrd = &input->str[input->cur_idx - input->wrd_idx];
    char limiting_char_backup = input->str[input->cur_idx];
    input->str[input->cur_idx] = '\0';
    if (*curr_wrd == '\0') {
        input->str[input->cur_idx] = limiting_char_backup;
        return no_matches;
    }
    if (in_1st_wrd(input) && there_is_no_slash_in_1st_wrd(input))
        /* PATH autocompletion */
        /* mutable reference of `match_str` to `ternary_search_tree` module */
        /* immutable reference of `input` to `ternary_search_tree` module */
        /* immutable reference of `path_tree` to `ternary_search_tree` module */
        res = tst_find_auto_completion_or_print_word_options(
            match_str, curr_wrd, path_tree, input->num_of_tab_key_presses, NULL
        );
    else
        res = filesystem_autocompletion_or_printing_word_options(
            match_str, curr_wrd, input->num_of_tab_key_presses
        );
    if ((res == more_than_one) && (input->num_of_tab_key_presses > 1))
        reprint_current_input(input);
    input->str[input->cur_idx] = limiting_char_backup;
    return res;
}

static void print_char_on_term(char c, type_input *input)
{
    putchar(c);
    if (input->cur_idx != input->end_idx) {
        reprint_the_shift_of_the_string_to_the_(right, input);
        return_cursor_back_n_positions(input->end_idx - input->cur_idx);
    }
}

static bool we_are_inside_1st_wrd(
    bool we_autocomplete_1st_wrd, const type_input *input
)
{
    return (we_autocomplete_1st_wrd && input->str[input->cur_idx] != ' ');
}

static void autocomplete_single_char(char c, type_input *input)
{
    print_char_on_term(c, input);
    add_character_to_str_arr(c, input->cur_idx, input);
    input->cur_idx++;
    input->end_idx++;
    input->wrd_idx++;
}

static bool match_str_doesnt_end_with_slash(
    const char *match_str, type_input *input
)
{
    return (match_str[input->wrd_idx - 1] != '/');
}

static void autocomplete_curr_wrd(
    type_input *input, char *match_str, autocomplete_matches_found matches_found
)
{
    bool we_autocomplete_1st_wrd = in_1st_wrd(input);
    if (we_are_inside_1st_wrd(we_autocomplete_1st_wrd, input))
        input->_1st_wrd_end_idx = input->cur_idx;
    while (match_str[input->wrd_idx]) {
        autocomplete_single_char(match_str[input->wrd_idx], input);
        if (we_autocomplete_1st_wrd)
            input->_1st_wrd_end_idx++;
        input->num_of_tab_key_presses = 0;
    }
    if (match_str_doesnt_end_with_slash(match_str, input) &&
        matches_found == just_one)
    {
        autocomplete_single_char(' ', input);
        input->wrd_idx = 0;
    }
    add_character_to_str_arr('\0', input->end_idx, input);
}

static void handle_eof(
    type_input *input, type_tokenizer *tknzer, type_tst *path_tree
)
{
#if defined(LOG)
    int res = write(log_fd, CTRL_D, sizeof(CTRL_D)-1);
    error_handling(res, __FILE__, __LINE__, "write");
#endif
    shut_down_my_shell(input, tknzer, path_tree);
}

static void handle_space(type_input *input)
{
#if defined(LOG)
    int res = write(log_fd, " ", 1);
    error_handling(res, __FILE__, __LINE__, "write");
#endif
    print_char_on_term(' ', input);
    add_character_to_str_arr(' ', input->cur_idx, input);
    input->cur_idx++;
    input->end_idx++;
    input->wrd_idx = 0;
    input->num_of_tab_key_presses = 0;
    add_character_to_str_arr('\0', input->end_idx, input);
}

static void handle_enter(type_input *input)
{
#if defined(LOG)
    int res = write(log_fd, ENTER, sizeof(ENTER)-1);
    error_handling(res, __FILE__, __LINE__, "write");
#endif
    putchar('\n');
    add_character_to_str_arr('\n', input->end_idx, input);
    input->end_idx++;
    add_character_to_str_arr('\0', input->end_idx, input);
    input->cur_idx = 0;
    input->end_idx = 0;
    input->wrd_idx = 0;
    input->num_of_tab_key_presses = 0;
}

static void handle_default_char(char c, type_input *input)
{
#if defined(LOG)
    int res = write(log_fd, &c, 1);
    error_handling(res, __FILE__, __LINE__, "write");
#endif
    print_char_on_term(c, input);
    add_character_to_str_arr(c, input->cur_idx, input);
    default_char_handle_1st_wrd(input);
    input->cur_idx++;
    input->end_idx++;
    input->wrd_idx++;
    input->num_of_tab_key_presses = 0;
    add_character_to_str_arr('\0', input->end_idx, input);
}

static void handle_tab(type_input *input, const type_tst *path_tree)
{
#if defined(LOG)
    int write_res = write(log_fd, TAB, sizeof(TAB)-1);
    error_handling(write_res, __FILE__, __LINE__, "write");
#endif
    autocomplete_matches_found res;
    char match_str[dir_name_buf_size];
    input->num_of_tab_key_presses++;
    res = find_auto_completion_or_print_word_options(
        match_str, input, path_tree
    );
    if (res == no_matches)
        return;
    if ((res == more_than_one && input->num_of_tab_key_presses == 1) ||
        (res == just_one))
    {
        /* autocomple till the last common character or
        autocomplete the whole world (if it was `just_one` match) */
        autocomplete_curr_wrd(input, match_str, res);
    }
}

static int find_new_wrd_idx(const type_input *input)
{
    int i = 0;
    if ((input->end_idx == 0) || (input->cur_idx == 0))
        return 0;
    while (true) {
        i++;
        if (input->str[input->cur_idx-i] == ' ')
            return i - 1;
        if (input->cur_idx - i == 0)
            return i;
    }
}

static void wrd_idx_decrement(type_input *input)
{
    if (input->wrd_idx > 0)
        input->wrd_idx--;
    if (input->wrd_idx == 0)
        input->wrd_idx = find_new_wrd_idx(input);
}

static void handle_backspace(type_input *input)
{
#if defined(LOG)
    int res = write(log_fd, BACKSPACE, sizeof(BACKSPACE)-1);
    error_handling(res, __FILE__, __LINE__, "write");
#endif
    if (input->cur_idx == 0)
        return;
    printf("\b \b");
    del_chr_handle_1st_wrd(backspace, input);
    input->cur_idx--;
    input->end_idx--;
    wrd_idx_decrement(input);
    input->num_of_tab_key_presses = 0;
    if (input->cur_idx != input->end_idx) {
        reprint_the_shift_of_the_string_to_the_(left, input);
        return_cursor_back_n_positions(input->end_idx - input->cur_idx);
        move_string_arr_to_the_(left, input);
    }
    add_character_to_str_arr('\0', input->end_idx, input);
}

static void handle_left_arrow_escape_sequence(type_input *input)
{
#if defined(LOG)
    int res = write(log_fd, LEFT_ARROW, sizeof(LEFT_ARROW)-1);
    error_handling(res, __FILE__, __LINE__, "write");
#endif
    if (input->cur_idx > 0) {
        putchar('\b');
        input->cur_idx--;
    }
    wrd_idx_decrement(input);
}

static void handle_right_arrow_escape_sequence(type_input *input)
{
#if defined(LOG)
    int res = write(log_fd, RIGHT_ARROW, sizeof(RIGHT_ARROW)-1);
    error_handling(res, __FILE__, __LINE__, "write");
#endif
    if (input->cur_idx < input->end_idx) {
        printf("\x1b[1C");
        if (input->str[input->cur_idx] == ' ')
            input->wrd_idx = 0;
        else
            input->wrd_idx++;
        input->cur_idx++;
    }
}

static void handle_delete_escape_sequence(type_input *input)
{
#if defined(LOG)
    int res = write(log_fd, DELETE, sizeof(DELETE)-1);
    error_handling(res, __FILE__, __LINE__, "write");
#endif
    if (input->cur_idx == input->end_idx)
        return;
    reprint_the_shift_of_the_string_to_the_(left, input);
    return_cursor_back_n_positions(input->end_idx - input->cur_idx - 1);
    del_chr_handle_1st_wrd(delete, input);
    move_string_arr_to_the_(left, input);
    input->end_idx--;
}

static void handle_esc_sequence(
    const char *input_buf, int *idx, type_input *input
)
{
    input->num_of_tab_key_presses = 0;
    (*idx)++;
    if (input_buf[*idx] == '[') {
        (*idx)++;
        if (input_buf[*idx] == 'D')
            handle_left_arrow_escape_sequence(input);
        else
        if (input_buf[*idx] == 'C')
            handle_right_arrow_escape_sequence(input);
        else
        if (0 == strncmp(input_buf + *idx, "3~", 2)) {
            handle_delete_escape_sequence(input);
            (*idx)++;
        }
    }
}

static int find_delete_position(const type_input *input)
{
    int prev_wrd_idx = 0, i = 0;
    type_input input_copy;
    if (input->wrd_idx != 0)
        return input->cur_idx - input->wrd_idx;
    input_copy = *input;
    do {
        i++;
        input_copy.cur_idx--;
        prev_wrd_idx = find_new_wrd_idx(&input_copy);
    } while ((prev_wrd_idx == 0) && (input_copy.cur_idx != 0));
    return input->cur_idx - prev_wrd_idx - i;
}

static void delete_part_of_string_before_cursor(
    type_input *input, int new_idx, int removed_len, int tail_len
)
{
    int i;
    return_cursor_back_n_positions(removed_len);
    printf("%s", input->str + input->cur_idx);
    for (i=0; i < removed_len; i++)
        putchar(' ');
    return_cursor_back_n_positions(input->end_idx - new_idx);
    memmove(input->str + new_idx, input->str + input->cur_idx, tail_len);
    input->end_idx = input->end_idx - removed_len;
    input->cur_idx = new_idx;
}

static void delete_last_word(type_input *input)
{                                                      /* include zero byte */
#if defined(LOG)
    int res = write(log_fd, CTRL_W, sizeof(CTRL_W)-1);
    error_handling(res, __FILE__, __LINE__, "write");
#endif
    int new_idx, removed_len, tail_len = input->end_idx - input->cur_idx + 1;
    input->num_of_tab_key_presses = 0;
    if (input->cur_idx == 0)
        return;
    new_idx = find_delete_position(input);
    removed_len = input->cur_idx - new_idx;
    del_chr_sequence_handle_1st_wrd(ctrl_w, input, removed_len);
    delete_part_of_string_before_cursor(input, new_idx, removed_len, tail_len);
    input->wrd_idx = find_new_wrd_idx(input);
}

static void delete_beginning_of_string(type_input *input)
{
#if defined(LOG)
    int res = write(log_fd, CTRL_U, sizeof(CTRL_U)-1);
    error_handling(res, __FILE__, __LINE__, "write");
#endif
    int new_idx = 0, removed_len = input->cur_idx;
    int tail_len = input->end_idx - input->cur_idx + 1;  /* include zero byte */
    input->num_of_tab_key_presses = 0;
    if (input->cur_idx == 0)
        return;
    del_chr_sequence_handle_1st_wrd(ctrl_u, input, removed_len);
    delete_part_of_string_before_cursor(input, new_idx, removed_len, tail_len);
    input->wrd_idx = 0;
}

static void process_input_character(
    const char *input_buf, int *idx, type_input *input,
    type_tokenizer *tknzer, type_tst *path_tree
)
{
    switch (input_buf[*idx]) {
        case ' ':
            handle_space(input);
            break;
        case '\t':
            handle_tab(input, path_tree);
            break;
        case '\n':
            handle_enter(input);
            break;
        case backspace:
            handle_backspace(input);
            break;
        case ctrl_d:
            handle_eof(input, tknzer, path_tree);
            break;
        case ctrl_w:
            delete_last_word(input);
            break;
        case ctrl_u:
            delete_beginning_of_string(input);
            break;
        case esc:
            handle_esc_sequence(input_buf, idx, input);
            break;
        default:
            handle_default_char(input_buf[*idx], input);
    }
}

void read_interactive_string(
    type_input *input, type_tokenizer *tknzer, type_tst *path_tree
)
{
    char input_buf[input_buf_size];
    int read_res, i;
    reset_input(input);
    printf("> ");
    fflush(stdout);
    while (
        (read_res = read_signal_protected(0, input_buf, input_buf_size)) != 0
    )
    {
        error_handling(read_res, __FILE__, __LINE__, "read");
        for (i=0; i < read_res; i++)
            process_input_character(input_buf, &i, input, tknzer, path_tree);
        fflush(stdout);
        if (input_buf[read_res-1] == '\n')
            return;
    }
    shut_down_my_shell(input, tknzer, path_tree);
}
