/* err_code.h */

#ifndef ERR_CODE_H_INCLUDED
#define ERR_CODE_H_INCLUDED

typedef enum tag_error_code {
    no_error,
    incorrect_char_escaping,
    unmatched_quotes,
    background_operator_not_in_the_end_of_str,
    separator_right_after_input_or_output_redirection,
    second_simple_word_right_after_input_or_output_redirecton,
    input_or_output_separator_used_in_line_twice,
    pipe_operator_at_start_of_str,
    not_implemented_feature
} error_code;

#endif
