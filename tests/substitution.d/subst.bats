#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

check_output_parser() {
    diff --strip-trailing-cr -uN "${1/parser/expected}.txt" --label "${1/parser/expected}" <(grep '^ *TEST ' < "$1") --label "output"
}

@test "Testing substitution.d - generation" {
    test_generate
}

@test "Testing substitution.d - header" {
    check_output_parser "substitution.d/parser.h"
}

@test "Testing substitution.d - source" {
    check_output_parser "substitution.d/parser.c"
}
