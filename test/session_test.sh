#!/usr/bin/env bash

# Test sequences - each sequence represents a continuous shell session
sequences=(
    # First test sequence
    "cd ..
cat file1.txt"
    # Add more test sequences as needed
    # "command1
    # command2
    # command3"
)

# Expected outputs after EACH command in the sequence
expected_step_outputs=(
    # Expected outputs for first sequence (one per command)
    $'\npipiska'
    # Add more expected step outputs for other sequences
)

# Run tests
passed=0
total=${#sequences[@]}

for idx in "${!sequences[@]}"; do
    # Split sequence into individual commands
    mapfile -t commands < <(printf "%s" "${sequences[idx]}")

    # Split expected outputs into steps
    mapfile -t expected_steps < <(printf "%s" "${expected_step_outputs[idx]}")

    # Check if we have matching number of commands and expected outputs
    if [[ ${#commands[@]} -ne ${#expected_steps[@]} ]]; then
        echo "ERROR: Number of commands (${#commands[@]}) doesn't match number of expected outputs (${#expected_steps[@]}) for sequence $((idx+1))"
        continue
    fi

    # Create a temporary script file
    temp_script=$(mktemp)

    # Run the sequence step by step for debugging if needed
    sequence_passed=true
    current_script=""
    partial_output=""

    for cmd_idx in "${!commands[@]}"; do
        # Add this command to our accumulating script
        current_script+="${commands[cmd_idx]}"$'\n'

        # Write to the temporary file the commands up to this point
        echo "$current_script" > "$temp_script"

        # Run shell with all commands up to this point
        partial_output=$(./my_shell < "$temp_script" 2>&1 | sed -e 's/> //g' -e 's/\^D$//')

        # Compare with expected output at this step
        if [[ "${expected_steps[cmd_idx]}" != "$partial_output" ]]; then
            sequence_passed=false

            printf -- '--------------------------------------------------------------------------------\n'
            printf 'STEP %d FAILED in sequence %d\n' "$((cmd_idx+1))" "$((idx+1))"
            printf 'Command: %s\n\n' "${commands[cmd_idx]}"
            printf 'Expected output up to this point:\n%b\n\n' "${expected_steps[cmd_idx]}"
            printf 'Actual output up to this point:\n%b\n\n' "$partial_output"

            # Show diff at this step
            wdiff <(echo "${expected_steps[cmd_idx]}") <(echo "$partial_output")
            printf '\n--------------------------------------------------------------------------------\n'

            # Option: break on first failure
            break
        fi
    done

    # Clean up temp file
    rm -f "$temp_script"

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
