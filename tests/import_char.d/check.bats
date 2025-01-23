#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

@test "Testing issue_85.d - generation" {
    run test_generate
    [ "$status" -eq 0 ]
}
