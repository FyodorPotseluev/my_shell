#!/usr/bin/env bash

set -o pipefail

valgrind_command="valgrind --error-exitcode=42 --tool=memcheck --leak-check=full --errors-for-leak-kinds=definite,indirect,possible --show-leak-kinds=definite,indirect,possible"

sequences=(
"mkdir dir
cd dir
echo one > dir_file.txt
cat dir_file.txt
echo one > background_work_test.txt &
cat background_work_test.txt
echo one > background_work_test_2.txt&
cat background_work_test.txt
echo two > dir_file.txt
cat dir_file.txt
../test/test_program < non_existing_file
../test/test_program < dir_file.txt
echo one >> dir_file_2.txt
cat dir_file_2.txt
echo two >> dir_file_2.txt
cat dir_file_2.txt
> dir_file_3.txt
cat dir_file_3.txt
> dir_file.txt
cat dir_file.txt
< non_existing_file
< dir_file_2.txt
cat dir_file_2.txt
>> dir_file_4.txt
cat dir_file_4.txt
>> dir_file_2.txt
cat dir_file_2.txt
../test/test_program < dir_file_2.txt > dir_file.txt
cat dir_file.txt
../test/test_program > dir_file.txt < dir_file_2.txt
cat dir_file.txt
../test/test_program > dir_file.txt < dir_file_2.txt &
cat dir_file.txt
> dir_file_2.txt
../test/test_program > dir_file_2.txt dir_file.txt
> ../test/test_program < dir_file.txt dir_file.txt
../test/test_program > dir_file_2.txt dir_file.txt blabla
../test/test_program < dir_file_2.txt dir_file.txt blabla
../test/test_program >> dir_file.txt dir_file.txt
../test/test_program >> dir_file.txt dir_file.txt blabla
../test/test_program > dir_file_2.txt > dir_file.txt
../test/test_program < dir_file.txt < dir_file_2.txt
../test/test_program >> dir_file_2.txt > dir_file.txt
../test/test_program > dir_file_2.txt >> dir_file.txt
../test/test_program > && dir_file_2.txt
../test/test_program>&&dir_file_2.txt
cat dir_file_2.txt
cat dir_file.txt
cd ..
rm -r dir"
)

tmp_dir=$(mktemp -d)
pipe="$tmp_dir/pipe"
mkfifo "$pipe"

fail_test_flag=0
total=${#sequences[@]}

for idx in "${!sequences[@]}"; do
    echo "Running memory test for sequence $((idx+1))/$total..."

    IFS=$'\n' read -d '' -ra commands <<< "${sequences[idx]}"
    total_commands=${#commands[@]}

    $valgrind_command ./build/bin/my_shell < "$pipe" > "$tmp_dir/output" 2> "$tmp_dir/valgrind_output" &
    shell_pid=$!

    exec 3>"$pipe"
    sleep 1  # Allow shell to initialize

    for ((cmd_idx=0; cmd_idx<total_commands; cmd_idx++)); do
        cmd="${commands[cmd_idx]}"
        printf "STEP %d/%d: %s\n" "$((cmd_idx+1))" "$total_commands" "$cmd"
        echo "$cmd" >&3
        sleep 1
    done

    exec 3>&-  # Close the pipe to send EOF

    wait $shell_pid
    exit_code=$?

    if [ $exit_code -eq 42 ]; then
        printf '%s\n' '------------------------------------------------------------------------------'
        printf 'Memory leaks detected in sequence %d\n' "$((idx+1))"
        printf '%s\n' '------------------------------------------------------------------------------'
        cat "$tmp_dir/valgrind_output"
        printf '%s\n' '------------------------------------------------------------------------------'
        fail_test_flag=1
    fi
done

rm -rf "$tmp_dir"

if [[ "$fail_test_flag" = 0 ]]; then
    echo "All memory tests passed successfully!"
else
    echo "Memory leaks detected in one or more test sequences."
    exit 1
fi
