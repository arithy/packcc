#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

@test "Testing code_line_continuation.d - generation" {
    run "$PACKCC" --debug -o "code_line_continuation.d/parser" "code_line_continuation.d/input.peg" 2>&1
    check_output "code_line_continuation.d/expected.txt"
}
