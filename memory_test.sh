#!/usr/bin/env bash

# if any command in the pipeline fails (returns a nonzero exit code), the
# pipeline's exit code will be the first nonzero exit code from left to right
set -o pipefail

# valgrind exitcode can't match program error codes: this causes the test to
# react in some cases when program returns error exitcode as to a test failure
valgrind_command="valgrind --error-exitcode=42 --tool=memcheck --leak-check=full --errors-for-leak-kinds=definite,indirect,possible --show-leak-kinds=definite,indirect,possible"

# project=./my_shell
# build_dir=./build
# bin_dir=$build_dir/bin
# executable=$bin_dir/$project

input=(
    "abra schwabra kadabra"
    "abra   "
    $'     abra\t\tschwabra \t \tkadabra'
    $'abra \"schwabra kadabra\" \"foo    bar\"'
    $'abra schw\"abra ka\"dab\"ra\" foo\"    \"bar'
    $'abra schwabra kadabra\"  foo bar'
    "abraschwabrakadabra"
    $'w\"  \"\"  \"ord'
    $'w\"o \"\" r\"d'
    "    "
    $' \t  \t  \t\t '
    $'\"\"'
    $'\"\"\"\"'
    $'\"\" \t\"\"'
    $' \"\" \t\"\" '
    $'\\\"abra\\\" \\\"schwabra\\\" \\\"kadabra\\\"'
    $'\\\\abra\\\\ \\\\schwabra\\\\ \\\\kadabra\\\\'
    $'abra \\\\s\\\"c\\\\h\\\"w\\\\a\\\"b\\\\r\\\"a\\\\ kadabra'
    $'abra \\\\schw\"abra\\\\\\\"ka\"dab\"ra\\\"\" foo\"    \"bar'
    $'abra \\\\schw\"a\\\\b\\\\ra\\\"k\\\"a\"dab\"ra\\\"\" foo\"    \"bar'
    $'abra \\\\schw\"a\\\\b\\\"ra\\\\k\\\"a\"dab\"ra\\\"\" foo\"    \"bar'
    $'word \"'
    $'abra schw\"abraka\"dab\"ra\"\\ foo\"    \"bar'
    $'abra schw\"abra ka\"dab\"r\\a\" foo\"    \"bar'
    $'abra schw\"abra\\ ka\"dab\"ra\" foo\"    \"bar'
    $'abra schw\"abra ka\"dab\"ra\" f\\oo\"    \"bar'
    $'abra schw\\\"abra ka\"dab\"ra\" foo\"    \"bar'
    # Simulate EOF with empty input
    ""
)

i=0
arr_len=${#input[@]}
fail_test_flag=0
for idx in "${!input[@]}"; do
# here-string `<<<` automatically appends EOF after sending `${input[idx]}`
# to `./my_shell.` . It happens because the program reaches the end of the
# temporary input stream (which was crteated by Bash), receives EOF and stops.

# sed -e 's/> //g' -e 's/\^D$//'
#   sed - pars and transform text
#   's/> //g'
#   s   - substitute `s/pattern/replacement/flags`
#   g   - global "apply to all occurrences in the line," not just the first one
#   's/\^D$//'
#   \^D - matches `^D` (EOF) character (every `input` element ends with EOF)
#   $   -  matches the end of the line, the character must be at the end of line
    output=$( $valgrind_command ./my_shell <<< "${input[idx]}" 2>&1 | sed -e 's/> //g' -e 's/\^D$//' )
    if [ $? -eq 42 ]; then
        printf '%s\n' \
            '------------------------------------------------------------------------------'
        printf '%s\n' "$arg"
        printf '%s\n' \
            '------------------------------------------------------------------------------'
        echo "$output"
        printf '%s\n' \
            '------------------------------------------------------------------------------'
        fail_test_flag=1
    fi
    i=$((i+1))
    printf '[%d/%d]\n' "$i" "$arr_len"
done
if [[ "$fail_test_flag" = 0 ]]; then
    echo OK
fi
