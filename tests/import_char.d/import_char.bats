#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

@test "Testing import_char.d - generation" {
    run test_generate
    [ "$status" -eq 0 ]
}
