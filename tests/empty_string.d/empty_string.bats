#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

@test "Testing empty_string.d - generation" {
    test_generate
}

@test "Testing empty_string.d - compilation" {
    test_compile
}
