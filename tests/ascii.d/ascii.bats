#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

@test "Testing $TEST_NAME - generation" {
    PACKCC_OPTS=("--ascii")
    test_generate
}

@test "Testing $TEST_NAME - check code" {
    ! in_source "pcc_get_char_as_utf32"
}

@test "Testing $TEST_NAME - compilation" {
    test_compile
}
@test "Testing $TEST_NAME - run" {
    run_for_input "$BATS_TEST_DIRNAME/input.txt"
}
