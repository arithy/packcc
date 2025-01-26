#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

check_output_parser() {
    diff --strip-trailing-cr -uN "${1/parser/expected}.txt" --label "${1/parser/expected}" <(grep 'TEST:' < "$1") --label "output"
}

@test "Testing code_indentation.d - generation" {
    test_generate
}

@test "Testing code_indentation.d - indentation" {
    check_output_parser "code_indentation.d/parser.c"
}
