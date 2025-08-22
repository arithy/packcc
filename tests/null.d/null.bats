#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

@test "Testing null.d - generation" {
    test_generate
}

@test "Testing null.d - compilation" {
    test_compile
}

@test "Testing null.d - run" {
    tr "X" "\0" < "$BATS_TEST_DIRNAME/input.txt" > "$BATS_TEST_DIRNAME/input.bin"
    run_for_input "$BATS_TEST_DIRNAME/input.bin" "$BATS_TEST_DIRNAME/expected.txt"
}
