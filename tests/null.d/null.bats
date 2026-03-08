#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

@test "Testing $TEST_NAME - generation" {
    test_generate
}

@test "Testing $TEST_NAME - compilation" {
    test_compile
}

@test "Testing $TEST_NAME - run" {
    tr "X" "\0" < "$BATS_TEST_DIRNAME/input.txt" > "$BATS_TEST_DIRNAME/input.bin"
    run_for_input "$BATS_TEST_DIRNAME/input.bin" "$BATS_TEST_DIRNAME/expected.txt"
}
