#!/usr/bin/env bash

input=()
expected=()

input+=( "abra schwabra kadabra" )
expected+=( $'[abra]\n[schwabra]\n[kadabra]' );

input+=( $'     abra\t\tschwabra \t \tkadabra' )
expected+=( $'[abra]\n[schwabra]\n[kadabra]' )

input+=( $'abra \"schwabra kadabra\" \"foo    bar\"' )
expected+=( $'[abra]\n[schwabra kadabra]\n[foo    bar]' )

input+=( $'abra schw\"abra ka\"dab\"ra\" foo\"    \"bar' )
expected+=( $'[abra]\n[schwabra kadabra]\n[foo    bar]' )

input+=( $'abra schwabra kadabra\"  foo bar' )
expected+=( $'Error: unmatched quotes' )

input+=( "abraschwabrakadabra" )
expected+=( "[abraschwabrakadabra]" )

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
