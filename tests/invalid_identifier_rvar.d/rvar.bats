#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

@test "Testing invalid_identifier_rvar.d - generation" {
    run test_generate
    [ "$status" -ne 0 ]
    [ $(echo "$output" | egrep "rule variable identifier '(_foo|pcc_foo|PCC_foo)'" | wc -l) == "3" ]
}
