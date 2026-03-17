#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

@test "Testing $TEST_NAME - generation" {
    test_generate
}

@test "Testing $TEST_NAME - compilation" {
    test_compile
}
