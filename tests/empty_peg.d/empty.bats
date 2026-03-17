#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

@test "Testing $TEST_NAME - generation [case 0]" {
    test_generate "$BATS_TEST_DIRNAME/input_0.peg"
}

@test "Testing $TEST_NAME - compilation [case 0]" {
    test_compile
}

@test "Testing $TEST_NAME - generation [case 1]" {
    test_generate "$BATS_TEST_DIRNAME/input_1.peg"
}

@test "Testing $TEST_NAME - compilation [case 1]" {
    test_compile
}
