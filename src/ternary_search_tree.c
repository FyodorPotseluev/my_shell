/* ternary_search_tree.c */

#include "ternary_search_tree.h"
#include "read_interactive_string.h"
#include "handle_err.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

enum consts {
    buf_len = 80,
    max_buf_idx = buf_len - 1
};

static void add_node(type_tst **curr_node, char chr)
{
    *curr_node = malloc(sizeof(type_tst));
    (*curr_node)->c = chr;
    (*curr_node)->end_of_word = false;
    (*curr_node)->less = (*curr_node)->equal = (*curr_node)->more = NULL;
}

static bool added_str_has_ended(const char *str)
{
    return (*(str + 1) == '\0');
}

static void add_to_tst(                 /* string idx */
    type_tst **curr_node, const char *str, int idx, bool it_is_dir_name
)
{
    if (*str == '\0')
        return;
    if (!*curr_node)
        add_node(curr_node, str[idx]);
    if (str[idx] == (*curr_node)->c) {
        if (added_str_has_ended(str+idx)) {
            if ((*curr_node)->c == '/')
                goto complete_adding;
            if (it_is_dir_name)
                add_to_tst(&(*curr_node)->equal, "/", 0, it_is_dir_name);
            else
                complete_adding:
                (*curr_node)->end_of_word = true;
            return;
        }
        add_to_tst(&(*curr_node)->equal, str, idx+1, it_is_dir_name);
    } else
    if (str[idx] < (*curr_node)->c)
        add_to_tst(&(*curr_node)->less, str, idx, it_is_dir_name);
    else
    if (str[idx] > (*curr_node)->c)
        add_to_tst(&(*curr_node)->more, str, idx, it_is_dir_name);
}

static char *get_path_directory_str(char *path, int *idx)
{
    /* path_str = "dir1:subdir/dir2:another_dir:more:dir:to:come"; */
    int init_idx = *idx;
    if (!path[*idx])
        return NULL;
    while (true) {
        if (path[*idx] == ':') {
            path[*idx] = '\0';
            (*idx)++;
            break;
        } else
        if (path[*idx] == '\0') {
            break;
        }
        (*idx)++;
    }
    return path + init_idx;
}

static bool is_it_dir_name(const char *str)
{
    int res;
    struct stat statbuf;
    if (!str)
        return false;
    res = stat(str, &statbuf);
    error_handling(res, __FILE__, __LINE__, "stat");
    if ((statbuf.st_mode & S_IFMT) == S_IFDIR)
        return true;
    else
        return false;
}

static char *get_full_file_name(
    char *dir_name, const char *file_name, int dir_name_end_idx
)
{
    int i = dir_name_end_idx;
    for (; *file_name; i++, file_name++) {
        if (i > dir_name_buf_size - 1) {
            fprintf(stderr, ERR_DIR_NAME_BUFFER_OVERFLOW, __FILE__, __LINE__);
            return NULL;
        }
        dir_name[i] = *file_name;
    }
    dir_name[i] = '\0';
    return dir_name;
}

void add_dir_files_names_to_tst(
    DIR *dir, type_tst **dir_files_tree, char *dir_name, int dir_name_end_idx
)
{
    bool it_is_dir_name = false;
    char *full_file_name = NULL;
    struct dirent *file = NULL;
    while ((file = readdir(dir))) {
        /* add every name to path array starting from the idx above ^^^ */
        if (dir_name) {
            full_file_name =
                get_full_file_name(dir_name, file->d_name, dir_name_end_idx);
        }
        it_is_dir_name = is_it_dir_name(full_file_name);
        add_to_tst(dir_files_tree, file->d_name, 0, it_is_dir_name);
    }
}

void init_path_tree(type_tst **path_tree)
{
    int res, idx = 0;
    const char *path_str = NULL;
    char *path_copy_str = NULL, *directory_str = NULL;
    DIR *directory = NULL;
    path_str = getenv("PATH");
    if (!path_str)
        return;
    path_copy_str = strdup(path_str);
    while ((directory_str = get_path_directory_str(path_copy_str, &idx))) {
        directory = opendir(directory_str);
        pointer_error_handling(directory, __FILE__, __LINE__, "opendir");
        add_dir_files_names_to_tst(directory, path_tree, NULL, 0);
        res = closedir(directory);
        error_handling(res, __FILE__, __LINE__, "closedir");
    }
    free(path_copy_str);
}

void free_tst(type_tst *curr_node)
{
    if (!curr_node)
        return;
    type_tst *tmp = curr_node;
    free_tst(curr_node->less);
    free_tst(curr_node->equal);
    free_tst(curr_node->more);
    free(tmp);
}

static void print_err_too_long_curr_wrd(const char *buf)
{
    fprintf(
        stderr,
        "%s:%d: The string \"%s\" exceeded the max buffer len, which is %d\n",
        __FILE__, __LINE__, buf, buf_len
    );
}

static bool it_is_completed_substring(const type_tst *curr_node)
{
    return (curr_node->end_of_word && curr_node->equal);
}

static bool there_is_more_than_one_autocompletion(const type_tst *curr_node)
{
    if ((curr_node->less) || (curr_node->more) ||
        it_is_completed_substring(curr_node))
    {
        return true;
    } else
        return false;
}

static void print_curr_word_if_it_is_complete(
    const type_tst *curr_node, char *buf, int idx
)
{
    if (curr_node->end_of_word) {
        buf[idx+1] = '\0';
        printf("%s    ", buf);
    }
}

static void print_word_options(const type_tst *curr_node, char *buf, int idx)
{
    if (!curr_node)
        return;
    if (idx == max_buf_idx) {
        buf[idx] = '\0';
        print_err_too_long_curr_wrd(buf);
        return;
    }
    print_word_options(curr_node->less, buf, idx);
    buf[idx] = curr_node->c;
    print_curr_word_if_it_is_complete(curr_node, buf, idx);
    print_word_options(curr_node->equal, buf, idx+1);
    print_word_options(curr_node->more, buf, idx);
}

static bool there_are_alternatives_to_curr_chr(const type_tst *curr_node)
{
    return ((curr_node->less || curr_node->more));
}

static void save_curr_autocompletion_or_print_words_options(
    const type_tst *curr_node, char *buf, int idx, int num_of_tab_key_presses
)
{
    if (num_of_tab_key_presses == 1) {
        /* save in `buf` autocompletion till the last common character */
        if (there_are_alternatives_to_curr_chr(curr_node))
            buf[idx] = '\0';
        else
            buf[idx+1] = '\0';
    } else
    if (num_of_tab_key_presses >=2) {
        putchar('\n');
        print_word_options(curr_node, buf, idx);
        putchar('\n');
    }
}

static autocomplete_matches_found
reqursive_call_tst_find_auto_completion_or_print_word_options(
    const type_tst *curr_node, char *buf, int idx, int num_of_tab_key_presses
)
{
    if (idx == max_buf_idx) {
        buf[idx] = '\0';
        print_err_too_long_curr_wrd(buf);
        return no_matches;
    }
    buf[idx] = curr_node->c;
    if (there_is_more_than_one_autocompletion(curr_node)) {
        save_curr_autocompletion_or_print_words_options(
            curr_node, buf, idx, num_of_tab_key_presses
        );
        return more_than_one;
    } else
    if (curr_node->end_of_word) {
        buf[idx+1] = '\0';
        return just_one;
    }
    return reqursive_call_tst_find_auto_completion_or_print_word_options(
        curr_node->equal, buf, idx+1, num_of_tab_key_presses
    );
}

static bool curr_wrd_ended(const char *curr_wrd, int idx)
{
    return (curr_wrd[idx+1] == '\0');
}

static bool found_curr_wrd_in_tst(
    const type_tst *curr_node, const type_tst **autocomplete_node,
    const char *curr_wrd, char *buf, int idx, int *autocomplete_idx
)
{
    if (!curr_node)
        return false;
    if (curr_wrd[idx] == '\0') {
        buf[idx] = '\0';
        goto curr_wrd_found;
    }
    if (idx >= max_buf_idx - 1) {
        print_err_too_long_curr_wrd(curr_wrd);
        return false;
    }
    if (found_curr_wrd_in_tst(curr_node->less, autocomplete_node, curr_wrd, buf,
        idx, autocomplete_idx))
    {
        return true;
    }
    if (curr_wrd[idx] == curr_node->c) {
        buf[idx] = curr_node->c;
        if (curr_wrd_ended(curr_wrd, idx)) {
            buf[idx+1] = '\0';
            curr_wrd_found:
            *autocomplete_node = curr_node;
            *autocomplete_idx = idx;
            return true;
        } else
        if (found_curr_wrd_in_tst(curr_node->equal, autocomplete_node, curr_wrd,
            buf, idx+1, autocomplete_idx))
        {
            return true;
        }
    }
    if (found_curr_wrd_in_tst(curr_node->more, autocomplete_node, curr_wrd, buf,
        idx, autocomplete_idx))
    {
        return true;
    }
    return false;
}

static bool overflow_check(int i, char *file_name, int line_num)
{
    if (i == dir_name_buf_size) {
        fprintf(stderr, ERR_DIR_NAME_BUFFER_OVERFLOW, file_name, line_num);
        return true;
    } else
        return false;
}

static void get_match_str(
    char *match_str, const type_path *path, const char *buf
)
{
    int i = 0;
    bool overflow;
    if (path) {
        for (; i < path->end_idx; i++) {
            overflow = overflow_check(i, __FILE__, __LINE__);
            if (overflow)
                return;
            match_str[i] = path->str[i];
        }
    }
    for (; *buf; i++, buf++) {
        overflow = overflow_check(i, __FILE__, __LINE__);
        if (overflow)
            return;
        match_str[i] = *buf;
    }
    match_str[i] = '\0';
}

static void refine_autocompl_vars(
    const char *buf, const type_tst **autocomplete_node, int *autocomplete_idx
)
{
    if (*buf) {
        *autocomplete_node = (*autocomplete_node)->equal;
        *autocomplete_idx += 1;
    }
}

autocomplete_matches_found tst_find_auto_completion_or_print_word_options(
    char *match_str, const char *curr_wrd, const type_tst *tree,
    int num_of_tab_key_presses, const type_path *path
)
{
    autocomplete_matches_found res;
    char buf[buf_len];
    int idx = 0, autocomplete_idx = 0;
    const type_tst *autocomplete_node = NULL;
    if (found_curr_wrd_in_tst(tree, &autocomplete_node, curr_wrd, buf, idx,
        &autocomplete_idx))
    {
        if (it_is_completed_substring(autocomplete_node)) {
            refine_autocompl_vars(buf, &autocomplete_node, &autocomplete_idx);
            buf[autocomplete_idx+1] = '\0';
            if (num_of_tab_key_presses == 1)
                get_match_str(match_str, path, buf);
            else
            if (num_of_tab_key_presses >= 2) {
                printf("\n%s    ", buf);
                print_word_options(autocomplete_node, buf, autocomplete_idx);
                putchar('\n');
            }
            return more_than_one;
        } else {
            refine_autocompl_vars(buf, &autocomplete_node, &autocomplete_idx);
            res = reqursive_call_tst_find_auto_completion_or_print_word_options(
                autocomplete_node, buf, autocomplete_idx, num_of_tab_key_presses
            );
            if ((res == more_than_one && num_of_tab_key_presses == 1) ||
                (res == just_one))
            {
                get_match_str(match_str, path, buf);
            }
            return res;
        }
    } else
        return no_matches;
}
