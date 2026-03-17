#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

@test "Testing $TEST_NAME - generation" {
    run "$PACKCC" --debug -o "$BATS_TEST_DIRNAME/parser" "$BATS_TEST_DIRNAME/input.peg" 2>&1
    check_output "$BATS_TEST_DIRNAME/expected.txt"
}
