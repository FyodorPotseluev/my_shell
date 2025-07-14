#!/usr/bin/env bash

input=()
expected=()

input+=( $'abra schwabra kadabra' )
expected+=( $'abra schwabra kadabra\n[abra]\n[schwabra]\n[kadabra]' );

input+=( "abra   " )
expected+=( $'abra   \n[abra]' )

input+=( $'     abra\t\tschwabra \t \tkadabra' )
expected+=( $'     abra\t\tschwabra \t \tkadabra\n[abra]\n[schwabra]\n[kadabra]' )

input+=( $'abra \"schwabra kadabra\" \"foo    bar\"' )
expected+=( $'abra \"schwabra kadabra\" \"foo    bar\"\n[abra]\n[schwabra kadabra]\n[foo    bar]' )

input+=( $'abra schw\"abra ka\"dab\"ra\" foo\"    \"bar' )
expected+=( $'abra schw\"abra ka\"dab\"ra\" foo\"    \"bar\n[abra]\n[schwabra kadabra]\n[foo    bar]' )

input+=( $'\\\"abra\\\" \\\"schwabra\\\" \\\"kadabra\\\"' )
expected+=( $'\\\"abra\\\" \\\"schwabra\\\" \\\"kadabra\\\"\n[\"abra\"]\n[\"schwabra\"]\n[\"kadabra\"]' );

input+=( $'\\\\abra\\\\ \\\\schwabra\\\\ \\\\kadabra\\\\' )
expected+=( $'\\\\abra\\\\ \\\\schwabra\\\\ \\\\kadabra\\\\\n[\\abra\\]\n[\\schwabra\\]\n[\\kadabra\\]' );

input+=( $'abra \\\\s\\\"c\\\\h\\\"w\\\\a\\\"b\\\\r\\\"a\\\\ kadabra' )
expected+=( $'abra \\\\s\\\"c\\\\h\\\"w\\\\a\\\"b\\\\r\\\"a\\\\ kadabra\n[abra]\n[\\s\"c\\h\"w\\a\"b\\r\"a\\]\n[kadabra]' );

input+=( $'abra \\\\schw\"abra\\\\\\\"ka\"dab\"ra\\\"\" foo\"    \"bar' )
expected+=( $'abra \\\\schw\"abra\\\\\\\"ka\"dab\"ra\\\"\" foo\"    \"bar\n[abra]\n[\\schwabra\\\"kadabra\"]\n[foo    bar]' )

input+=( $'abra \\\\schw\"a\\\\b\\\\ra\\\"k\\\"a\"dab\"ra\\\"\" foo\"    \"bar' )
expected+=( $'abra \\\\schw\"a\\\\b\\\\ra\\\"k\\\"a\"dab\"ra\\\"\" foo\"    \"bar\n[abra]\n[\\schwa\\b\\ra\"k\"adabra\"]\n[foo    bar]' )

input+=( $'abra \\\\schw\"a\\\\b\\\"ra\\\\k\\\"a\"dab\"ra\\\"\" foo\"    \"bar' )
expected+=( $'abra \\\\schw\"a\\\\b\\\"ra\\\\k\\\"a\"dab\"ra\\\"\" foo\"    \"bar\n[abra]\n[\\schwa\\b\"ra\\k\"adabra\"]\n[foo    bar]' )

input+=( $'abra schwabra kadabra\"  foo bar' )
expected+=( $'abra schwabra kadabra\"  foo bar\nmy_shell: Error: unmatched quotes' )

input+=( $'word \"' )
expected+=( $'word \"\nmy_shell: Error: unmatched quotes' )

input+=( $'abra schw\"abraka\"dab\"ra\"\\ foo\"    \"bar' )
expected+=( $'abra schw\"abraka\"dab\"ra\"\\ foo\"    \"bar\nmy_shell: Error: only the characters `\"` and `\\` can be escaped' )

input+=( $'abra schw\"abra ka\"dab\"r\\a\" foo\"    \"bar' )
expected+=( $'abra schw\"abra ka\"dab\"r\\a\" foo\"    \"bar\nmy_shell: Error: only the characters `\"` and `\\` can be escaped' )

input+=( $'abra schw\"abra\\ ka\"dab\"ra\" foo\"    \"bar' )
expected+=( $'abra schw\"abra\\ ka\"dab\"ra\" foo\"    \"bar\nmy_shell: Error: only the characters `\"` and `\\` can be escaped' )

input+=( $'abra schw\"abra ka\"dab\"ra\" f\\oo\"    \"bar' )
expected+=( $'abra schw\"abra ka\"dab\"ra\" f\\oo\"    \"bar\nmy_shell: Error: only the characters `\"` and `\\` can be escaped' )

input+=( $'abra schw\\\"abra ka\"dab\"ra\" foo\"    \"bar' )
expected+=( $'abra schw\\\"abra ka\"dab\"ra\" foo\"    \"bar\nmy_shell: Error: unmatched quotes' )

input+=( "abraschwabrakadabra" )
expected+=( $'abraschwabrakadabra\n[abraschwabrakadabra]' )

input+=( $'w\"  \"\"  \"ord' )
expected+=( $'w\"  \"\"  \"ord\n[w    ord]' )

input+=( $'w\"o \"\" r\"d' )
expected+=( $'w\"o \"\" r\"d\n[wo  rd]' )

input+=( "    " )
expected+=( $'    ' )

input+=( $' \t  \t  \t\t ' )
expected+=( $' \t  \t  \t\t ' )

input+=( $'\"\"' )
expected+=( $'\"\"\n[]' )

input+=( $'\"\"\"\"' )
expected+=( $'\"\"\"\"\n[]' )

input+=( $'\"\" \t\"\"' )
expected+=( $'\"\" \t\"\"\n[]\n[]' )

input+=( $' \"\" \t\"\" ' )
expected+=( $' \"\" \t\"\" \n[]\n[]' )

input+=( $'\"\" word1 word2' )
expected+=( $'\"\" word1 word2\n[]\n[word1]\n[word2]' )

input+=( $'word1 \"\" word2' )
expected+=( $'word1 \"\" word2\n[word1]\n[]\n[word2]' )

input+=( $'word1 word2 \"\" word3' )
expected+=( $'word1 word2 \"\" word3\n[word1]\n[word2]\n[]\n[word3]' )

input+=( $'word1 word2 \"\"' )
expected+=( $'word1 word2 \"\"\n[word1]\n[word2]\n[]' )

input+=( $'word1 \"\"word2' )
expected+=( $'word1 \"\"word2\n[word1]\n[word2]' )

input+=( $'word1 \"\"word2' )
expected+=( $'word1 \"\"word2\n[word1]\n[word2]' )

input+=( $'w\"  \"\"  \"ord' )
expected+=( $'w\"  \"\"  \"ord\n[w    ord]' )

input+=( $'w\"o \"\" r\"d' )
expected+=( $'w\"o \"\" r\"d\n[wo  rd]' )

input+=( $'word \"\"' )
expected+=( $'word \"\"\n[word]\n[]' )

input+=( $'a \"It is a super long string, you see, I could actually overcome the bug where I unfortunately missed the issue that my tmp_wrd_array size was doubled only once, instead of being doubled every time the index value equals array size - 1. So lets see if everything is fine now.\" b' )
expected+=( $'a \"It is a super long string, you see, I could actually overcome the bug where I unfortunately missed the issue that my tmp_wrd_array size was doubled only once, instead of being doubled every time the index value equals array size - 1. So lets see if everything is fine now.\" b\n[a]\n[It is a super long string, you see, I could actually overcome the bug where I unfortunately missed the issue that my tmp_wrd_array size was doubled only once, instead of being doubled every time the index value equals array size - 1. So lets see if everything is fine now.]\n[b]' )

# Separators work check
input+=( "a & b" )
expected+=( $'a & b\n[a]\n[background_operator]\n[b]' )

input+=( "a&b" )
expected+=( $'a&b\n[a]\n[background_operator]\n[b]' )

input+=( "a && b" )
expected+=( $'a && b\n[a]\n[and_operator]\n[b]' )

input+=( "a&&b" )
expected+=( $'a&&b\n[a]\n[and_operator]\n[b]' )

input+=( "a &&& b" )
expected+=( $'a &&& b\n[a]\n[and_operator]\n[background_operator]\n[b]' )

input+=( "a&&&b" )
expected+=( $'a&&&b\n[a]\n[and_operator]\n[background_operator]\n[b]' )

input+=( "a &&&& b" )
expected+=( $'a &&&& b\n[a]\n[and_operator]\n[and_operator]\n[b]' )

input+=( "a&&&&b" )
expected+=( $'a&&&&b\n[a]\n[and_operator]\n[and_operator]\n[b]' )

input+=( "a &&&&& b" )
expected+=( $'a &&&&& b\n[a]\n[and_operator]\n[and_operator]\n[background_operator]\n[b]' )

input+=( "a&&&&&b" )
expected+=( $'a&&&&&b\n[a]\n[and_operator]\n[and_operator]\n[background_operator]\n[b]' )

input+=( "&" )
expected+=( $'&\n[background_operator]' )

input+=( "&&" )
expected+=( $'&&\n[and_operator]' )

input+=( "&&&" )
expected+=( $'&&&\n[and_operator]\n[background_operator]' )

input+=( "&&&&" )
expected+=( $'&&&&\n[and_operator]\n[and_operator]' )

input+=( "&&&&&" )
expected+=( $'&&&&&\n[and_operator]\n[and_operator]\n[background_operator]' )

input+=( "a\"&\"b" )
expected+=( $'a\"&\"b\n[a&b]' )

input+=( "a \"&\" b" )
expected+=( $'a \"&\" b\n[a]\n[&]\n[b]' )

input+=( "a \\& b" )
expected+=( $'a \\& b\nmy_shell: Error: only the characters `\"` and `\\` can be escaped' )

input+=( "\\&" )
expected+=( $'\\&\nmy_shell: Error: only the characters `\"` and `\\` can be escaped' )

# ------------------------------------------------------------------------------
input+=( "a > b" )
expected+=( $'a > b\n[a]\n[output_redirection]\n[b]' )

input+=( "a>b" )
expected+=( $'a>b\n[a]\n[output_redirection]\n[b]' )

input+=( "a >> b" )
expected+=( $'a >> b\n[a]\n[output_append_redirection]\n[b]' )

input+=( "a>>b" )
expected+=( $'a>>b\n[a]\n[output_append_redirection]\n[b]' )

input+=( "a >>> b" )
expected+=( $'a >>> b\n[a]\n[output_append_redirection]\n[output_redirection]\n[b]' )

input+=( "a>>>b" )
expected+=( $'a>>>b\n[a]\n[output_append_redirection]\n[output_redirection]\n[b]' )

input+=( "a >>>> b" )
expected+=( $'a >>>> b\n[a]\n[output_append_redirection]\n[output_append_redirection]\n[b]' )

input+=( "a>>>>b" )
expected+=( $'a>>>>b\n[a]\n[output_append_redirection]\n[output_append_redirection]\n[b]' )

input+=( "a >>>>> b" )
expected+=( $'a >>>>> b\n[a]\n[output_append_redirection]\n[output_append_redirection]\n[output_redirection]\n[b]' )

input+=( "a>>>>>b" )
expected+=( $'a>>>>>b\n[a]\n[output_append_redirection]\n[output_append_redirection]\n[output_redirection]\n[b]' )

input+=( ">" )
expected+=( $'>\n[output_redirection]' )

input+=( ">>" )
expected+=( $'>>\n[output_append_redirection]' )

input+=( ">>>" )
expected+=( $'>>>\n[output_append_redirection]\n[output_redirection]' )

input+=( ">>>>" )
expected+=( $'>>>>\n[output_append_redirection]\n[output_append_redirection]' )

input+=( ">>>>>" )
expected+=( $'>>>>>\n[output_append_redirection]\n[output_append_redirection]\n[output_redirection]' )

input+=( "a\">\"b" )
expected+=( $'a\">\"b\n[a>b]' )

input+=( "a \">\" b" )
expected+=( $'a \">\" b\n[a]\n[>]\n[b]' )

input+=( "a \\> b" )
expected+=( $'a \\> b\nmy_shell: Error: only the characters `\"` and `\\` can be escaped' )

input+=( "\\>" )
expected+=( $'\\>\nmy_shell: Error: only the characters `\"` and `\\` can be escaped' )

# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------
input+=( "a | b" )
expected+=( $'a | b\n[a]\n[pipe_operator]\n[b]' )

input+=( "a|b" )
expected+=( $'a|b\n[a]\n[pipe_operator]\n[b]' )

input+=( "a || b" )
expected+=( $'a || b\n[a]\n[or_operator]\n[b]' )

input+=( "a||b" )
expected+=( $'a||b\n[a]\n[or_operator]\n[b]' )

input+=( "a ||| b" )
expected+=( $'a ||| b\n[a]\n[or_operator]\n[pipe_operator]\n[b]' )

input+=( "a|||b" )
expected+=( $'a|||b\n[a]\n[or_operator]\n[pipe_operator]\n[b]' )

input+=( "a |||| b" )
expected+=( $'a |||| b\n[a]\n[or_operator]\n[or_operator]\n[b]' )

input+=( "a||||b" )
expected+=( $'a||||b\n[a]\n[or_operator]\n[or_operator]\n[b]' )

input+=( "a ||||| b" )
expected+=( $'a ||||| b\n[a]\n[or_operator]\n[or_operator]\n[pipe_operator]\n[b]' )

input+=( "a|||||b" )
expected+=( $'a|||||b\n[a]\n[or_operator]\n[or_operator]\n[pipe_operator]\n[b]' )

input+=( "|" )
expected+=( $'|\n[pipe_operator]' )

input+=( "||" )
expected+=( $'||\n[or_operator]' )

input+=( "|||" )
expected+=( $'|||\n[or_operator]\n[pipe_operator]' )

input+=( "||||" )
expected+=( $'||||\n[or_operator]\n[or_operator]' )

input+=( "|||||" )
expected+=( $'|||||\n[or_operator]\n[or_operator]\n[pipe_operator]' )

input+=( "a\"|\"b" )
expected+=( $'a\"|\"b\n[a|b]' )

input+=( "a \"|\" b" )
expected+=( $'a \"|\" b\n[a]\n[|]\n[b]' )

input+=( "a \\| b" )
expected+=( $'a \\| b\nmy_shell: Error: only the characters `\"` and `\\` can be escaped' )

input+=( "\\|" )
expected+=( $'\\|\nmy_shell: Error: only the characters `\"` and `\\` can be escaped' )

# ------------------------------------------------------------------------------

input+=( "a < b" )
expected+=( $'a < b\n[a]\n[input_redirection]\n[b]' )

input+=( "a<b" )
expected+=( $'a<b\n[a]\n[input_redirection]\n[b]' )

input+=( "a << b" )
expected+=( $'a << b\n[a]\n[input_redirection]\n[input_redirection]\n[b]' )

input+=( "a<<b" )
expected+=( $'a<<b\n[a]\n[input_redirection]\n[input_redirection]\n[b]' )

input+=( "<" )
expected+=( $'<\n[input_redirection]' )

input+=( "<<" )
expected+=( $'<<\n[input_redirection]\n[input_redirection]' )

input+=( "a\"<\"b" )
expected+=( $'a\"<\"b\n[a<b]' )

input+=( "a \"<\" b" )
expected+=( $'a \"<\" b\n[a]\n[<]\n[b]' )

input+=( "a \\< b" )
expected+=( $'a \\< b\nmy_shell: Error: only the characters `\"` and `\\` can be escaped' )

input+=( "\\<" )
expected+=( $'\\<\nmy_shell: Error: only the characters `\"` and `\\` can be escaped' )

# ------------------------------------------------------------------------------

input+=( "a ; b" )
expected+=( $'a ; b\n[a]\n[command_separator]\n[b]' )

input+=( "a;b" )
expected+=( $'a;b\n[a]\n[command_separator]\n[b]' )

input+=( "a ;; b" )
expected+=( $'a ;; b\n[a]\n[command_separator]\n[command_separator]\n[b]' )

input+=( "a;;b" )
expected+=( $'a;;b\n[a]\n[command_separator]\n[command_separator]\n[b]' )

input+=( ";" )
expected+=( $';\n[command_separator]' )

input+=( ";;" )
expected+=( $';;\n[command_separator]\n[command_separator]' )

input+=( "a\";\"b" )
expected+=( $'a\";\"b\n[a;b]' )

input+=( "a \";\" b" )
expected+=( $'a \";\" b\n[a]\n[;]\n[b]' )

# ------------------------------------------------------------------------------

input+=( "a ( b" )
expected+=( $'a ( b\n[a]\n[open_parenthesis]\n[b]' )

input+=( "a(b" )
expected+=( $'a(b\n[a]\n[open_parenthesis]\n[b]' )

input+=( "a (( b" )
expected+=( $'a (( b\n[a]\n[open_parenthesis]\n[open_parenthesis]\n[b]' )

input+=( "a((b" )
expected+=( $'a((b\n[a]\n[open_parenthesis]\n[open_parenthesis]\n[b]' )

input+=( "(" )
expected+=( $'(\n[open_parenthesis]' )

input+=( "((" )
expected+=( $'((\n[open_parenthesis]\n[open_parenthesis]' )

input+=( "a\"(\"b" )
expected+=( $'a\"(\"b\n[a(b]' )

input+=( "a \"(\" b" )
expected+=( $'a \"(\" b\n[a]\n[(]\n[b]' )

input+=( "a \\( b" )
expected+=( $'a \\( b\nmy_shell: Error: only the characters `\"` and `\\` can be escaped' )

input+=( "\\(" )
expected+=( $'\\(\nmy_shell: Error: only the characters `\"` and `\\` can be escaped' )

# ------------------------------------------------------------------------------

input+=( "a ) b" )
expected+=( $'a ) b\n[a]\n[close_parenthesis]\n[b]' )

input+=( "a)b" )
expected+=( $'a)b\n[a]\n[close_parenthesis]\n[b]' )

input+=( "a )) b" )
expected+=( $'a )) b\n[a]\n[close_parenthesis]\n[close_parenthesis]\n[b]' )

input+=( "a))b" )
expected+=( $'a))b\n[a]\n[close_parenthesis]\n[close_parenthesis]\n[b]' )

input+=( ")" )
expected+=( $')\n[close_parenthesis]' )

input+=( "))" )
expected+=( $'))\n[close_parenthesis]\n[close_parenthesis]' )

input+=( "a\")\"b" )
expected+=( $'a\")\"b\n[a)b]' )

input+=( "a \")\" b" )
expected+=( $'a \")\" b\n[a]\n[)]\n[b]' )

input+=( "a \\) b" )
expected+=( $'a \\) b\nmy_shell: Error: only the characters `\"` and `\\` can be escaped' )

input+=( "\\)" )
expected+=( $'\\)\nmy_shell: Error: only the characters `\"` and `\\` can be escaped' )

input+=( "&>" )
expected+=( $'&>\n[background_operator]\n[output_redirection]' )

input+=( "&>|&" )
expected+=( $'&>|&\n[background_operator]\n[output_redirection]\n[pipe_operator]\n[background_operator]' )

input+=( $'&\"&\"&' )
expected+=( $'&\"&\"&\n[background_operator]\n[&]\n[background_operator]' )

input+=( $'\"&>\"|&' )
expected+=( $'\"&>\"|&\n[&>]\n[pipe_operator]\n[background_operator]' )

input+=( "<;()" )
expected+=( $'<;()\n[input_redirection]\n[command_separator]\n[open_parenthesis]\n[close_parenthesis]' )

input+=( $'<\";(\")' )
expected+=( $'<\";(\")\n[input_redirection]\n[;(]\n[close_parenthesis]' )

# Simulate EOF with empty input
input+=( "" )
expected+=( "" )

i=0
arr_len=${#input[@]}
for idx in "${!input[@]}"; do
# here-string `<<<` automatically appends EOF after sending `${input[idx]}`
# to `./my_shell.`

# sed -e 's/> //' -e 's/\^D$//'
#   sed - pars and transform text
#   's/> //'
#   s   - substitute `s/pattern/replacement/flags`
#   's/\^D$//'
#   \^D - matches `^D` (EOF) character (every `input` element ends with EOF)
#   $   -  matches the end of the line, the character must be at the end of line
    actual=$(./build/bin/my_shell <<< "${input[idx]}" 2>&1 | sed -e 's/> //' -e 's/\^D$//')
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
