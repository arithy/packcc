#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

check_output_error() {
    diff --strip-trailing-cr -uN "$1" --label "$1" <(echo "$output" | sed -e 's/^packcc\.exe/packcc/') --label "output"
}

@test "Testing $TEST_NAME - generation" {
    test_generate
}

@test "Testing $TEST_NAME - generation [invalid case]" {
    run test_generate error.peg
    [ "$status" -ne 0 ]
    check_output_error "$BATS_TEST_DIRNAME/error_expected.txt"
}
