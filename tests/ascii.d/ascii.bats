#!/usr/bin/env bats

load $TESTDIR/utils.sh

in_source() {
    grep -Fq "$1" "ascii.d/parser.c"
}

@test "Testing ascii.d - generation" {
    PACKCC_OPTS=("--ascii")
    test_generate "ascii.d"
}

@test "Testing ascii.d - check code" {
    ! in_source "pcc_get_char_as_utf32"
}

@test "Testing ascii.d - compilation" {
    test_compile "ascii.d"
}
@test "Testing ascii.d - run" {
    run_for_input "ascii.d/input.txt"
}
