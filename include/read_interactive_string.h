/* read_interactive_string.h */

#ifndef READ_INTERACTIVE_STRING_H_INCLUDED
#define READ_INTERACTIVE_STRING_H_INCLUDED

#include "input.h"
#include "ternary_search_tree.h"
#include "tokenizer.h"

#define ERR_DIR_NAME_BUFFER_OVERFLOW \
    "Error: dir_name buffer overflow: %s:%d\n"

enum read_interactive_string_consts {
    dir_name_buf_size = 256     /* the longest possible directory name path */
};

void read_interactive_string(
    type_input *input, type_tokenizer *tknzer, type_tst *path_tree
);
/*
    Read the input formed by an interactive user until either the `\n` character
is read (flushes the input), or the `Ctrl + D` keyboard shortcut is pressed
(terminates the program). The input may contain significant ASCII characters,
Tab, Enter, and Backspace. It can also include the Delete key, the left and
right arrow keys, and the keyboard shortcuts `Ctrl + W`, `Ctrl + U`, and
`Ctrl + D`.
RECEIVES:
    - `input` the address of the `type_input` structure;
    - `tknzer` the address of the `type_tokenizer` structure (to free the memory
in case of the program termination);
    - `path_tree` the address of the `type_tst` structure that contains the
ternary search tree storing all the `PATH` file names */

#endif
