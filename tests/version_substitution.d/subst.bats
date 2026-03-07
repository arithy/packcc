#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

check_output_parser() {
    diff --strip-trailing-cr -uN "${1/parser/expected}.txt" --label "${1/parser/expected}" <(grep '^ *TEST ' < "$1") --label "output"
}

@test "Testing version_substitution.d - generation" {
    test_generate
}

@test "Testing version_substitution.d - header" {
    check_output_parser "$BATS_TEST_DIRNAME/parser.h"
}

@test "Testing version_substitution.d - source" {
    check_output_parser "$BATS_TEST_DIRNAME/parser.c"
}
