#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

@test "Testing $TEST_NAME - generation" {
    run test_generate
    [ "$status" -eq 10 ]
    [[ "$output" =~ ': input.peg:1:1: Illegal rule syntax' ]]
}
