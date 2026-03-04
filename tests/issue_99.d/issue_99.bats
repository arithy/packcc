#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

@test "Testing issue_99.d - generation" {
    test_generate
}

@test "Testing issue_99.d - compilation" {
    test_compile
}
