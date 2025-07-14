/* ternary_search_tree.h */

#ifndef TERNARY_SEARCH_TREE_H_INCLUDED
#define TERNARY_SEARCH_TREE_H_INCLUDED

#include <dirent.h>
#include <stdbool.h>

typedef struct type_ternary_search_tree {
    char c;
    bool end_of_word;
    struct type_ternary_search_tree *less;
    struct type_ternary_search_tree *equal;
    struct type_ternary_search_tree *more;
} type_tst;

typedef struct tag_type_path {
    const char *str;
    int end_idx;
} type_path;
/*
    The structure is used for file system name autocompletion and stores
the path (the actual user input up to and including the last '/') to the current
 file name.
    - `end_idx` indicates where the string ends and an ordinary string has `\0`
*/

void init_path_tree(type_tst **path_tree);
/*
    Scans all the user's PATH directories and adds their contents to the
`path_tree` ternary search tree */

void free_tst(type_tst *tree);
/*
    Frees the memory allocated for a ternary search tree */

void add_dir_files_names_to_tst(
    DIR *dir, type_tst **dir_files_tree, char *dir_name, int dir_name_end_idx
);
/*
    Adds the directory file names to ternary search tree:
RECEIVES:
    - `dir` an opened directory stream;
    - `dir_files_tree` the pointer of a variable where the tst will be stored;
    - `dir_name` the correct path to current directory (starts with "/" or "./")
    - `dir_name_end_idx` the `dir_name` null terminating byte index */

typedef enum tag_autocomplete_matches_found {
    no_matches,
    just_one,
    more_than_one
} autocomplete_matches_found;

autocomplete_matches_found tst_find_auto_completion_or_print_word_options(
    char *match_str, const char *curr_wrd, const type_tst *tree,
    int num_of_tab_key_presses, const type_path *path
);
/*
    The function gets the `curr_wrd` string and searches if it matches any of the strings stored in the `tree`.
    If not, it returns the `no_matches` value.
    If it does match, the function checks for how many strings in the `tree` start with the `curr_wrd` substring.
        - If there is the only one match, the function writes the match string into the `match_str` array and returns the `just_one` value.
        - If there is more than one match, the function returns the `more_than_one` value. Depending on the `num_of_tab_key_presses`:
            -- (for the `num_of_tab_key_presses` equal to 1) it returnes the `match_str` containing the `curr_wrd` characters + the characters up to the last common one of all the stored strings with the same prefix substring;
            -- (for the `num_of_tab_key_presses` greater than 1) it prints a list of match strings with the same `curr_wrd` prefix;
    The `path` variable is used when working with file system names. When working with the PATH names, we pass it as NULL. The returned `match_str` starts with the `path` string. */

#endif
