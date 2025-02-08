#!/usr/bin/env bash

input=()
expected=()

input+=( $'abra schwabra kadabra\n' )
expected+=( $'[abra]\n[schwabra]\n[kadabra]' );

input+=( $'abra   \n' )
expected+=( $'[abra]' );

input+=( $'     abra\t\tschwabra \t \tkadabra\n' )
expected+=( $'[abra]\n[schwabra]\n[kadabra]' )

input+=( $'abra \"schwabra kadabra\" \"foo    bar\"\n' )
expected+=( $'[abra]\n[schwabra kadabra]\n[foo    bar]' )

input+=( $'abra schw\"abra ka\"dab\"ra\" foo\"    \"bar\n' )
expected+=( $'[abra]\n[schwabra kadabra]\n[foo    bar]' )

input+=( $'\\\"abra\\\" \\\"schwabra\\\" \\\"kadabra\\\"\n' )
expected+=( $'[\"abra\"]\n[\"schwabra\"]\n[\"kadabra\"]' );

input+=( $'\\\\abra\\\\ \\\\schwabra\\\\ \\\\kadabra\\\\\n' )
expected+=( $'[\\abra\\]\n[\\schwabra\\]\n[\\kadabra\\]' );

input+=( $'abra \\\\s\\\"c\\\\h\\\"w\\\\a\\\"b\\\\r\\\"a\\\\ kadabra\n' )
expected+=( $'[abra]\n[\\s\"c\\h\"w\\a\"b\\r\"a\\]\n[kadabra]' );

input+=( $'abra \\\\schw\"abra\\\\\\\"ka\"dab\"ra\\\"\" foo\"    \"bar\n' )
expected+=( $'[abra]\n[\\schwabra\\\"kadabra\"]\n[foo    bar]' )

input+=( $'abra \\\\schw\"a\\\\b\\\\ra\\\"k\\\"a\"dab\"ra\\\"\" foo\"    \"bar\n' )
expected+=( $'[abra]\n[\\schwa\\b\\ra\"k\"adabra\"]\n[foo    bar]' )

input+=( $'abra \\\\schw\"a\\\\b\\\"ra\\\\k\\\"a\"dab\"ra\\\"\" foo\"    \"bar\n' )
expected+=( $'[abra]\n[\\schwa\\b\"ra\\k\"adabra\"]\n[foo    bar]' )

input+=( $'\"\" word1 word2\n' )
expected+=( $'[]\n[word1]\n[word2]' )

input+=( $'word1 \"\" word2\n' )
expected+=( $'[word1]\n[]\n[word2]' )

input+=( $'word1 word2 \"\" word3\n' )
expected+=( $'[word1]\n[word2]\n[]\n[word3]' )

input+=( $'word1 word2 \"\"\n' )
expected+=( $'[word1]\n[word2]\n[]' )

input+=( $'word1\"\" word2\n' )
expected+=( $'[word1]\n[word2]' )

input+=( $'word1 \"\"word2\n' )
expected+=( $'[word1]\n[word2]' )

input+=( $'word1 \"\"word2\n' )
expected+=( $'[word1]\n[word2]' )

input+=( $'w\"  \"\"  \"ord\n' )
expected+=( $'[w    ord]' )

input+=( $'w\"o \"\" r\"d\n' )
expected+=( $'[wo  rd]' )

input+=( $'word \"\"\n' )
expected+=( $'[word]\n[]' )

input+=( $'word \"\n' )
expected+=( $'Error: unmatched quotes' )

input+=( $'abra schw\"abraka\"dab\"ra\"\\ foo\"    \"bar\n' )
expected+=( $'Error: only the characters `\"` and `\\` can be escaped' )

input+=( $'abra schw\"abra ka\"dab\"r\\a\" foo\"    \"bar\n' )
expected+=( $'Error: only the characters `\"` and `\\` can be escaped' )

input+=( $'abra schw\"abra\\ ka\"dab\"ra\" foo\"    \"bar\n' )
expected+=( $'Error: only the characters `\"` and `\\` can be escaped' )

input+=( $'abra schw\"abra ka\"dab\"ra\" f\\oo\"    \"bar\n' )
expected+=( $'Error: only the characters `\"` and `\\` can be escaped' )

input+=( $'abra schw\\\"abra ka\"dab\"ra\" foo\"    \"bar\n' )
expected+=( $'Error: unmatched quotes' )

input+=( $'abra schwabra kadabra\"  foo bar\n' )
expected+=( $'Error: unmatched quotes' )

input+=( $'abraschwabrakadabra\n' )
expected+=( "[abraschwabrakadabra]" )

input+=( "    " )
expected+=( "" )

input+=( $' \t  \t  \t\t ' )
expected+=( "" )

input+=( $'\"\"' )
expected+=( "" )

input+=( $'\"\"\"\"' )
expected+=( "" )

input+=( $'\"\" \t\"\"' )
expected+=( "" )

input+=( $' \"\" \t\"\" ' )
expected+=( "" )

# Simulate EOF with empty input
input+=( "" )
expected+=( "" )

i=0
arr_len=${#input[@]}
for idx in "${!input[@]}"; do
# sed -e 's/> //g' -e 's/\^D$//'
#   sed - pars and transform text
#   's/> //g'
#   s   - substitute `s/pattern/replacement/flags`
#   g   - global "apply to all occurrences in the line," not just the first one
#   's/\^D$//'
#   \^D - matches `^D` (EOF) character (every `input` element ends with EOF)
#   $   -  matches the end of the line, the character must be at the end of line
    actual=$(./my_shell <<< "${input[idx]}" | sed -e 's/> //g' -e 's/\^D$//')
    if [[ "${expected[idx]}" != "$actual" ]]; then
        printf -- '--------------------------------------------------------------------------------\n'
        printf '\nTEST "%b" FAILED: expected: \n%b\ngot: \n%b\n\n' \
          "${input[idx]}" \
          "${expected[idx]}" \
          "$actual"
# wdiff is diff command analogue that shows the exact place with difference
        wdiff <(echo "${expected[idx]}" ) <(echo "$actual")
        printf '\n--------------------------------------------------------------------------------'
    else
        i=$((i+1))
    fi
done
if [[ "$i" = "$arr_len" ]]; then
    echo OK
fi
