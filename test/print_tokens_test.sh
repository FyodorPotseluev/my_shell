#!/usr/bin/env bash

input=()
expected=()

input+=( $'abra schwabra kadabra\n' )
expected+=( $'[abra]\n[schwabra]\n[kadabra]' );

input+=( "abra   " )
expected+=( $'[abra]' )

input+=( $'     abra\t\tschwabra \t \tkadabra' )
expected+=( $'[abra]\n[schwabra]\n[kadabra]' )

input+=( $'abra \"schwabra kadabra\" \"foo    bar\"\n' )
expected+=( $'[abra]\n[schwabra kadabra]\n[foo    bar]' )

input+=( $'abra schw\"abra ka\"dab\"ra\" foo\"    \"bar\n' )
expected+=( $'[abra]\n[schwabra kadabra]\n[foo    bar]' )

input+=( $'\\\"abra\\\" \\\"schwabra\\\" \\\"kadabra\\\"' )
expected+=( $'[\"abra\"]\n[\"schwabra\"]\n[\"kadabra\"]' );

input+=( $'\\\\abra\\\\ \\\\schwabra\\\\ \\\\kadabra\\\\' )
expected+=( $'[\\abra\\]\n[\\schwabra\\]\n[\\kadabra\\]' );

input+=( $'abra \\\\s\\\"c\\\\h\\\"w\\\\a\\\"b\\\\r\\\"a\\\\ kadabra' )
expected+=( $'[abra]\n[\\s\"c\\h\"w\\a\"b\\r\"a\\]\n[kadabra]' );

input+=( $'abra \\\\schw\"abra\\\\\\\"ka\"dab\"ra\\\"\" foo\"    \"bar' )
expected+=( $'[abra]\n[\\schwabra\\\"kadabra\"]\n[foo    bar]' )

input+=( $'abra \\\\schw\"a\\\\b\\\\ra\\\"k\\\"a\"dab\"ra\\\"\" foo\"    \"bar' )
expected+=( $'[abra]\n[\\schwa\\b\\ra\"k\"adabra\"]\n[foo    bar]' )

input+=( $'abra \\\\schw\"a\\\\b\\\"ra\\\\k\\\"a\"dab\"ra\\\"\" foo\"    \"bar' )
expected+=( $'[abra]\n[\\schwa\\b\"ra\\k\"adabra\"]\n[foo    bar]' )

input+=( $'abra schwabra kadabra\"  foo bar' )
expected+=( $'my_shell: Error: unmatched quotes' )

input+=( $'word \"' )
expected+=( $'my_shell: Error: unmatched quotes' )

input+=( $'abra schw\"abraka\"dab\"ra\"\\ foo\"    \"bar' )
expected+=( $'my_shell: Error: only the characters `\"` and `\\` can be escaped' )

input+=( $'abra schw\"abra ka\"dab\"r\\a\" foo\"    \"bar' )
expected+=( $'my_shell: Error: only the characters `\"` and `\\` can be escaped' )

input+=( $'abra schw\"abra\\ ka\"dab\"ra\" foo\"    \"bar' )
expected+=( $'my_shell: Error: only the characters `\"` and `\\` can be escaped' )

input+=( $'abra schw\"abra ka\"dab\"ra\" f\\oo\"    \"bar' )
expected+=( $'my_shell: Error: only the characters `\"` and `\\` can be escaped' )

input+=( $'abra schw\\\"abra ka\"dab\"ra\" foo\"    \"bar' )
expected+=( $'my_shell: Error: unmatched quotes' )

input+=( "abraschwabrakadabra" )
expected+=( "[abraschwabrakadabra]" )

input+=( $'w\"  \"\"  \"ord' )
expected+=( $'[w    ord]' )

input+=( $'w\"o \"\" r\"d' )
expected+=( $'[wo  rd]' )

input+=( "    " )
expected+=( "" )

input+=( $' \t  \t  \t\t ' )
expected+=( "" )

input+=( $'\"\"' )
expected+=( "[]" )

input+=( $'\"\"\"\"' )
expected+=( "[]" )

input+=( $'\"\" \t\"\"' )
expected+=( $'[]\n[]' )

input+=( $' \"\" \t\"\" ' )
expected+=( $'[]\n[]' )

input+=( $'\"\" word1 word2' )
expected+=( $'[]\n[word1]\n[word2]' )

input+=( $'word1 \"\" word2' )
expected+=( $'[word1]\n[]\n[word2]' )

input+=( $'word1 word2 \"\" word3' )
expected+=( $'[word1]\n[word2]\n[]\n[word3]' )

input+=( $'word1 word2 \"\"' )
expected+=( $'[word1]\n[word2]\n[]' )

input+=( $'word1 \"\"word2' )
expected+=( $'[word1]\n[word2]' )

input+=( $'word1 \"\"word2' )
expected+=( $'[word1]\n[word2]' )

input+=( $'w\"  \"\"  \"ord' )
expected+=( $'[w    ord]' )

input+=( $'w\"o \"\" r\"d' )
expected+=( $'[wo  rd]' )

input+=( $'word \"\"' )
expected+=( $'[word]\n[]' )

input+=( $'a \"It is a super long string, you see, I could actually overcome the bug where I unfortunately missed the issue that my tmp_wrd_array size was doubled only once, instead of being doubled every time the index value equals array size - 1. So lets see if everything is fine now.\" b' )
expected+=( $'[a]\n[It is a super long string, you see, I could actually overcome the bug where I unfortunately missed the issue that my tmp_wrd_array size was doubled only once, instead of being doubled every time the index value equals array size - 1. So lets see if everything is fine now.]\n[b]' )

# Separators work check
input+=( "a & b" )
expected+=( $'[a]\n[background_operator]\n[b]' )

input+=( "a&b" )
expected+=( $'[a]\n[background_operator]\n[b]' )

input+=( "a && b" )
expected+=( $'[a]\n[and_operator]\n[b]' )

input+=( "a&&b" )
expected+=( $'[a]\n[and_operator]\n[b]' )

input+=( "a &&& b" )
expected+=( $'[a]\n[and_operator]\n[background_operator]\n[b]' )

input+=( "a&&&b" )
expected+=( $'[a]\n[and_operator]\n[background_operator]\n[b]' )

input+=( "a &&&& b" )
expected+=( $'[a]\n[and_operator]\n[and_operator]\n[b]' )

input+=( "a&&&&b" )
expected+=( $'[a]\n[and_operator]\n[and_operator]\n[b]' )

input+=( "a &&&&& b" )
expected+=( $'[a]\n[and_operator]\n[and_operator]\n[background_operator]\n[b]' )

input+=( "a&&&&&b" )
expected+=( $'[a]\n[and_operator]\n[and_operator]\n[background_operator]\n[b]' )

input+=( "&" )
expected+=( $'[background_operator]' )

input+=( "&&" )
expected+=( $'[and_operator]' )

input+=( "&&&" )
expected+=( $'[and_operator]\n[background_operator]' )

input+=( "&&&&" )
expected+=( $'[and_operator]\n[and_operator]' )

input+=( "&&&&&" )
expected+=( $'[and_operator]\n[and_operator]\n[background_operator]' )

input+=( "a\"&\"b" )
expected+=( $'[a&b]' )

input+=( "a \"&\" b" )
expected+=( $'[a]\n[&]\n[b]' )

input+=( "a \\& b" )
expected+=( $'my_shell: Error: only the characters `\"` and `\\` can be escaped' )

input+=( "\\&" )
expected+=( $'my_shell: Error: only the characters `\"` and `\\` can be escaped' )

# ------------------------------------------------------------------------------
input+=( "a > b" )
expected+=( $'[a]\n[output_redirection]\n[b]' )

input+=( "a>b" )
expected+=( $'[a]\n[output_redirection]\n[b]' )

input+=( "a >> b" )
expected+=( $'[a]\n[output_append_redirection]\n[b]' )

input+=( "a>>b" )
expected+=( $'[a]\n[output_append_redirection]\n[b]' )

input+=( "a >>> b" )
expected+=( $'[a]\n[output_append_redirection]\n[output_redirection]\n[b]' )

input+=( "a>>>b" )
expected+=( $'[a]\n[output_append_redirection]\n[output_redirection]\n[b]' )

input+=( "a >>>> b" )
expected+=( $'[a]\n[output_append_redirection]\n[output_append_redirection]\n[b]' )

input+=( "a>>>>b" )
expected+=( $'[a]\n[output_append_redirection]\n[output_append_redirection]\n[b]' )

input+=( "a >>>>> b" )
expected+=( $'[a]\n[output_append_redirection]\n[output_append_redirection]\n[output_redirection]\n[b]' )

input+=( "a>>>>>b" )
expected+=( $'[a]\n[output_append_redirection]\n[output_append_redirection]\n[output_redirection]\n[b]' )

input+=( ">" )
expected+=( $'[output_redirection]' )

input+=( ">>" )
expected+=( $'[output_append_redirection]' )

input+=( ">>>" )
expected+=( $'[output_append_redirection]\n[output_redirection]' )

input+=( ">>>>" )
expected+=( $'[output_append_redirection]\n[output_append_redirection]' )

input+=( ">>>>>" )
expected+=( $'[output_append_redirection]\n[output_append_redirection]\n[output_redirection]' )

input+=( "a\">\"b" )
expected+=( $'[a>b]' )

input+=( "a \">\" b" )
expected+=( $'[a]\n[>]\n[b]' )

input+=( "a \\> b" )
expected+=( $'my_shell: Error: only the characters `\"` and `\\` can be escaped' )

input+=( "\\>" )
expected+=( $'my_shell: Error: only the characters `\"` and `\\` can be escaped' )

# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------
input+=( "a | b" )
expected+=( $'[a]\n[pipe_operator]\n[b]' )

input+=( "a|b" )
expected+=( $'[a]\n[pipe_operator]\n[b]' )

input+=( "a || b" )
expected+=( $'[a]\n[or_operator]\n[b]' )

input+=( "a||b" )
expected+=( $'[a]\n[or_operator]\n[b]' )

input+=( "a ||| b" )
expected+=( $'[a]\n[or_operator]\n[pipe_operator]\n[b]' )

input+=( "a|||b" )
expected+=( $'[a]\n[or_operator]\n[pipe_operator]\n[b]' )

input+=( "a |||| b" )
expected+=( $'[a]\n[or_operator]\n[or_operator]\n[b]' )

input+=( "a||||b" )
expected+=( $'[a]\n[or_operator]\n[or_operator]\n[b]' )

input+=( "a ||||| b" )
expected+=( $'[a]\n[or_operator]\n[or_operator]\n[pipe_operator]\n[b]' )

input+=( "a|||||b" )
expected+=( $'[a]\n[or_operator]\n[or_operator]\n[pipe_operator]\n[b]' )

input+=( "|" )
expected+=( $'[pipe_operator]' )

input+=( "||" )
expected+=( $'[or_operator]' )

input+=( "|||" )
expected+=( $'[or_operator]\n[pipe_operator]' )

input+=( "||||" )
expected+=( $'[or_operator]\n[or_operator]' )

input+=( "|||||" )
expected+=( $'[or_operator]\n[or_operator]\n[pipe_operator]' )

input+=( "a\"|\"b" )
expected+=( $'[a|b]' )

input+=( "a \"|\" b" )
expected+=( $'[a]\n[|]\n[b]' )

input+=( "a \\| b" )
expected+=( $'my_shell: Error: only the characters `\"` and `\\` can be escaped' )

input+=( "\\|" )
expected+=( $'my_shell: Error: only the characters `\"` and `\\` can be escaped' )

# ------------------------------------------------------------------------------

input+=( "a < b" )
expected+=( $'[a]\n[input_redirection]\n[b]' )

input+=( "a<b" )
expected+=( $'[a]\n[input_redirection]\n[b]' )

input+=( "a << b" )
expected+=( $'[a]\n[input_redirection]\n[input_redirection]\n[b]' )

input+=( "a<<b" )
expected+=( $'[a]\n[input_redirection]\n[input_redirection]\n[b]' )

input+=( "<" )
expected+=( $'[input_redirection]' )

input+=( "<<" )
expected+=( $'[input_redirection]\n[input_redirection]' )

input+=( "a\"<\"b" )
expected+=( $'[a<b]' )

input+=( "a \"<\" b" )
expected+=( $'[a]\n[<]\n[b]' )

input+=( "a \\< b" )
expected+=( $'my_shell: Error: only the characters `\"` and `\\` can be escaped' )

input+=( "\\<" )
expected+=( $'my_shell: Error: only the characters `\"` and `\\` can be escaped' )

# ------------------------------------------------------------------------------

input+=( "a ; b" )
expected+=( $'[a]\n[command_separator]\n[b]' )

input+=( "a;b" )
expected+=( $'[a]\n[command_separator]\n[b]' )

input+=( "a ;; b" )
expected+=( $'[a]\n[command_separator]\n[command_separator]\n[b]' )

input+=( "a;;b" )
expected+=( $'[a]\n[command_separator]\n[command_separator]\n[b]' )

input+=( ";" )
expected+=( $'[command_separator]' )

input+=( ";;" )
expected+=( $'[command_separator]\n[command_separator]' )

input+=( "a\";\"b" )
expected+=( $'[a;b]' )

input+=( "a \";\" b" )
expected+=( $'[a]\n[;]\n[b]' )

# ------------------------------------------------------------------------------

input+=( "a ( b" )
expected+=( $'[a]\n[open_parenthesis]\n[b]' )

input+=( "a(b" )
expected+=( $'[a]\n[open_parenthesis]\n[b]' )

input+=( "a (( b" )
expected+=( $'[a]\n[open_parenthesis]\n[open_parenthesis]\n[b]' )

input+=( "a((b" )
expected+=( $'[a]\n[open_parenthesis]\n[open_parenthesis]\n[b]' )

input+=( "(" )
expected+=( $'[open_parenthesis]' )

input+=( "((" )
expected+=( $'[open_parenthesis]\n[open_parenthesis]' )

input+=( "a\"(\"b" )
expected+=( $'[a(b]' )

input+=( "a \"(\" b" )
expected+=( $'[a]\n[(]\n[b]' )

input+=( "a \\( b" )
expected+=( $'my_shell: Error: only the characters `\"` and `\\` can be escaped' )

input+=( "\\(" )
expected+=( $'my_shell: Error: only the characters `\"` and `\\` can be escaped' )

# ------------------------------------------------------------------------------

input+=( "a ) b" )
expected+=( $'[a]\n[close_parenthesis]\n[b]' )

input+=( "a)b" )
expected+=( $'[a]\n[close_parenthesis]\n[b]' )

input+=( "a )) b" )
expected+=( $'[a]\n[close_parenthesis]\n[close_parenthesis]\n[b]' )

input+=( "a))b" )
expected+=( $'[a]\n[close_parenthesis]\n[close_parenthesis]\n[b]' )

input+=( ")" )
expected+=( $'[close_parenthesis]' )

input+=( "))" )
expected+=( $'[close_parenthesis]\n[close_parenthesis]' )

input+=( "a\")\"b" )
expected+=( $'[a)b]' )

input+=( "a \")\" b" )
expected+=( $'[a]\n[)]\n[b]' )

input+=( "a \\) b" )
expected+=( $'my_shell: Error: only the characters `\"` and `\\` can be escaped' )

input+=( "\\)" )
expected+=( $'my_shell: Error: only the characters `\"` and `\\` can be escaped' )

input+=( "&>" )
expected+=( $'[background_operator]\n[output_redirection]' )

input+=( "&>|&" )
expected+=( $'[background_operator]\n[output_redirection]\n[pipe_operator]\n[background_operator]' )

input+=( $'&\"&\"&' )
expected+=( $'[background_operator]\n[&]\n[background_operator]' )

input+=( $'\"&>\"|&' )
expected+=( $'[&>]\n[pipe_operator]\n[background_operator]' )

input+=( "<;()" )
expected+=( $'[input_redirection]\n[command_separator]\n[open_parenthesis]\n[close_parenthesis]' )

input+=( $'<\";(\")' )
expected+=( $'[input_redirection]\n[;(]\n[close_parenthesis]' )

# Simulate EOF with empty input
input+=( "" )
expected+=( "" )

i=0
arr_len=${#input[@]}
for idx in "${!input[@]}"; do
# here-string `<<<` automatically appends EOF after sending `${input[idx]}`
# to `./my_shell.`

# sed -e 's/> //g' -e 's/\^D$//'
#   sed - pars and transform text
#   's/> //g'
#   s   - substitute `s/pattern/replacement/flags`
#   g   - global "apply to all occurrences in the line," not just the first one
#   's/\^D$//'
#   \^D - matches `^D` (EOF) character (every `input` element ends with EOF)
#   $   -  matches the end of the line, the character must be at the end of line
    actual=$(./build/bin/my_shell <<< "${input[idx]}" 2>&1 | sed -e 's/> //g' -e 's/\^D$//')
    if [[ "${expected[idx]}" != "$actual" ]]; then
        printf -- '--------------------------------------------------------------------------------\n'
        printf '\nTEST\n%b\nFAILED: expected: \n%b\ngot: \n%b\n\n' \
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
