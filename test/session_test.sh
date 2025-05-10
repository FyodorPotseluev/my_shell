#!/usr/bin/env bash
# Test sequences - each sequence represents a continuous shell session
sequences=(
    # Test sequence
    # Includes:
    # The correct work of:
    #       `> non_existing_file`       (create `non_existing_file`);
    #       `> existing_file`           (clear `existing_file`);
    #       `< non_existing_file`       (error);
    #       `< existing_file`           (nothing happens);
    #       `>> non_existing_file`      (create `non_existing_file`);
    #       `>> existing_file`          (nothing happens);
    #
    #       `cmd > non_existing_file`   (create `non_existing_file` w content);
    #       `cmd > existing_file`       (clear `existing_file`, write content);
    #       `cmd < non_existing_file`   (error);
    #       `cmd < existing_file`       (output as it was `stdin`);
    #       `cmd >> non_existing_file`  (create `non_existing_file` w content);
    #       `cmd >> existing_file`      (append `existing_file`);
    # You see the same separator in a line twice: Error;
    # You see `>` and `>>` separators in one line: Error;
    # You see a separator after the redirection token: Error;
    # You see more than one simple word after a separator redirecton: Error;
    # Separators redirection combination (one input, one output): Correct work;
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
cat ../LICENSE.txt | ../test/test_program | head -n 3
cat ../LICENSE.txt | ../test/test_program | head -n 3 | grep 2024
cat ../LICENSE.txt | cat | cat | ../test/test_program | cat | head -n 3 | cat | grep 2024
cat < ../LICENSE.txt | cat | cat | ../test/test_program | cat | head -n 3 | cat | grep 2024
cd ..
rm -r dir"
)

# Expected outputs after EACH command in the sequence
expected_outputs=(
    # mkdir dir
    ""
    # cd dir
    ""
    # echo one > dir_file.txt
    ""
    # cat dir_file.txt
    "one"
    # echo one > background_work_test.txt &
    ""
    # cat background_work_test.txt
    "one"
    # echo one > background_work_test_2.txt&
    ""
    # cat background_work_test.txt
    "one"
    # echo two > dir_file.txt
    ""
    # cat dir_file.txt
    "two"
    # ../test/test_program < non_existing_file
    "my_shell: non_existing_file: No such file or directory"
    # ../test/test_program < dir_file.txt
    "(two)"
    # echo one >> dir_file_2.txt
    ""
    # cat dir_file_2.txt
    "one"
    # echo two >> dir_file_2.txt
    ""
    # cat dir_file_2.txt
    $'one\ntwo'
    # > dir_file_3.txt
    ""
    # cat dir_file_3.txt
    ""
    # > dir_file.txt
    ""
    # cat dir_file.txt
    ""
    # < non_existing_file
    "my_shell: non_existing_file: No such file or directory"
    # < dir_file_2.txt
    ""
    # cat dir_file_2.txt
    $'one\ntwo'
    # >> dir_file_4.txt
    ""
    # cat dir_file_4.txt
    ""
    # >> dir_file_2.txt
    ""
    # cat dir_file_2.txt
    $'one\ntwo'
    # ../test/test_program < dir_file_2.txt > dir_file.txt
    ""
    # cat dir_file.txt
    $'(one)\n(two)'
    # ../test/test_program > dir_file.txt < dir_file_2.txt
    ""
    # cat dir_file.txt
    $'(one)\n(two)'
    # ../test/test_program > dir_file.txt < dir_file_2.txt &
    ""
    # cat dir_file.txt
    $'(one)\n(two)'
    # > dir_file_2.txt
    ""
    # ../test/test_program > dir_file_2.txt dir_file.txt
    "my_shell: Error: 2nd file name after IO redirection"
    # > ../test/test_program < dir_file.txt dir_file.txt
    "my_shell: Error: 2nd file name after IO redirection"
    # ../test/test_program > dir_file_2.txt dir_file.txt blabla
    "my_shell: Error: 2nd file name after IO redirection"
    # ../test/test_program < dir_file_2.txt dir_file.txt blabla
    "my_shell: Error: 2nd file name after IO redirection"
    # ../test/test_program >> dir_file.txt dir_file.txt
    "my_shell: Error: 2nd file name after IO redirection"
    # ../test/test_program >> dir_file.txt dir_file.txt blabla
    "my_shell: Error: 2nd file name after IO redirection"
    # ../test/test_program > dir_file_2.txt > dir_file.txt
    "my_shell: Error: > or < used twice, or > together with >>"
    # ../test/test_program < dir_file.txt < dir_file_2.txt
    "my_shell: Error: > or < used twice, or > together with >>"
    # ../test/test_program >> dir_file_2.txt > dir_file.txt
    "my_shell: Error: > or < used twice, or > together with >>"
    # ../test/test_program > dir_file_2.txt >> dir_file.txt
    "my_shell: Error: > or < used twice, or > together with >>"
    # ../test/test_program > && dir_file_2.txt
    "my_shell: Error: separator right after IO redirection"
    # ../test/test_program>&&dir_file_2.txt
    "my_shell: Error: separator right after IO redirection"
    # cat dir_file_2.txt
    ""
    # cat dir_file.txt
    $'(one)\n(two)'
    # cat ../LICENSE.txt | ../test/test_program | head -n 3
    $'(MIT) (License)\n\n(Copyright) ((c)) (2024) (FyodorPotseluev)'
    # cat ../LICENSE.txt | ../test/test_program | head -n 3 | grep 2024
    "(Copyright) ((c)) (2024) (FyodorPotseluev)"
    # cat ../LICENSE.txt | cat | cat | ../test/test_program | cat | head -n 3 | cat | grep 2024
    "(Copyright) ((c)) (2024) (FyodorPotseluev)"
    # cat < ../LICENSE.txt | cat | cat | ../test/test_program | cat | head -n 3 | cat | grep 2024
    "(Copyright) ((c)) (2024) (FyodorPotseluev)"
    # cd ..
    ""
    # rm -r dir
    ""
)
# Run tests
passed=0
total=${#sequences[@]}
for idx in "${!sequences[@]}"; do
    # Split sequence into individual commands
    IFS=$'\n' read -d '' -ra commands <<< "${sequences[idx]}"

    # Calculate the offset for expected outputs
    expected_start=0
    for i in $(seq 0 $((idx-1))); do
        # Get number of commands in previous sequence
        IFS=$'\n' read -d '' -ra prev_commands <<< "${sequences[i]}"
        expected_start=$((expected_start + ${#prev_commands[@]}))
    done

    expected_end=$((expected_start + ${#commands[@]} - 1))
    if [[ $expected_end -ge ${#expected_outputs[@]} ]]; then
        echo "ERROR: Not enough expected outputs for sequence $((idx+1)). Need ${#commands[@]} outputs, starting at index $expected_start."
        continue
    fi

    # Start a single shell process for the entire sequence
    # Use a named pipe for input
    pipe=$(mktemp -u)
    mkfifo "$pipe"

    # Use a temporary file to store output from each command
    output_file=$(mktemp)

    # Start the shell in the background, reading from the pipe
    ./build/bin/my_shell < "$pipe" > /tmp/shell_output 2>&1 &
    shell_pid=$!

    # Open the pipe for writing
    exec 3>"$pipe"

    # Wait for the shell to initialize
    sleep 0.2
    # Clear the initial prompt
    > /tmp/shell_output

    # Run the sequence step by step
    sequence_passed=true

    echo "Starting test sequence $((idx+1))..."
    for cmd_idx in "${!commands[@]}"; do
        current_cmd="${commands[cmd_idx]}"

        # Send the command to the shell
        echo "$current_cmd" >&3

        # Give the shell a moment to process
        sleep 0.5

        # Get the output from this command
        output=$(cat /tmp/shell_output | tr -d '\0' | sed -e 's/^> //' -e 's/\^D$//' | tr -d '\r')

        # Clear the output file for the next command
        > /tmp/shell_output

        # Get the expected output for this step
        expected_idx=$((expected_start + cmd_idx))
        expected="${expected_outputs[expected_idx]}"

        # Trim trailing whitespace from both
        output=$(echo "$output" | sed 's/[[:space:]]*$//')
        expected=$(echo "$expected" | sed 's/[[:space:]]*$//')

        # Compare output with expected output
        if [[ "$output" == "$expected" ]]; then
            printf 'STEP %d/%d PASSED: %s\n' "$((cmd_idx+1))" "${#commands[@]}" "$current_cmd"
        else
            sequence_passed=false
            printf -- '--------------------------------------------------------------------------------\n'
            printf 'STEP %d FAILED in sequence %d\n' "$((cmd_idx+1))" "$((idx+1))"
            printf 'Command: %s\n\n' "$current_cmd"
            printf 'Expected output:\n%b\n\n' "$expected"
            printf 'Actual output:\n%b\n\n' "$output"

            # Show diff
            echo "Diff:"
            wdiff <(echo "$expected") <(echo "$output") || true
            printf '\n--------------------------------------------------------------------------------\n'
            break
        fi
    done

    # Close the pipe
    exec 3>&-

    # Send EOF to the shell and wait for it to exit
    echo -e "\004" > "$pipe"
    wait $shell_pid 2>/dev/null || true

    # Clean up
    rm -f "$pipe" /tmp/shell_output "$output_file"

    if $sequence_passed; then
        passed=$((passed+1))
        printf 'Test sequence %d: PASSED\n' "$((idx+1))"
    fi

    printf '\n'
done
if [[ "$passed" = "$total" ]]; then
    echo "All $total test sequences PASSED"
else
    echo "$passed out of $total test sequences PASSED"
fi
