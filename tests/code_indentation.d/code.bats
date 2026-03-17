#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

check_output_parser() {
    diff --strip-trailing-cr -uN "${1/parser/expected}.txt" --label "${1/parser/expected}" <(grep 'TEST:' < "$1") --label "output"
}

@test "Testing $TEST_NAME - generation" {
    test_generate
}

@test "Testing $TEST_NAME - indentation" {
    check_output_parser "$BATS_TEST_DIRNAME/parser.c"
}
