#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

@test "Testing $TEST_NAME - generation" {
    run test_generate
    [ "$status" -ne 0 ]
    [ $(echo "$output" | egrep "marker variable identifier '(_foo|pcc_foo|PCC_foo)'" | wc -l) == "3" ]
}
