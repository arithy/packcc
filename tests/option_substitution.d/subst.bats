#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

make_expected() {
    PACKCC_VER=$("$PACKCC" --version | grep 'version' | sed -e 's/^.* version \([^ ]*\) */\1/')
    sed -e "s/PACKCC_VER/${PACKCC_VER}/g" < "$1.h.txt" > "${1/expected_?/expected}.h.txt"
    sed -e "s/PACKCC_VER/${PACKCC_VER}/g" < "$1.c.txt" > "${1/expected_?/expected}.c.txt"
}

check_output_parser() {
    diff --strip-trailing-cr -uN "${1/parser/expected}.txt" --label "${1/parser/expected}" <(grep '^ *TEST ' < "$1") --label "output"
}

@test "Testing $TEST_NAME - generation [case 0]" {
    test_generate
    make_expected "$BATS_TEST_DIRNAME/expected_0"
}

@test "Testing $TEST_NAME - header [case 0]" {
    check_output_parser "$BATS_TEST_DIRNAME/parser.h"
}

@test "Testing $TEST_NAME - source [case 0]" {
    check_output_parser "$BATS_TEST_DIRNAME/parser.c"
}

@test "Testing $TEST_NAME - generation [case 1]" {
    PACKCC_OPTS=("--ascii")
    test_generate
    make_expected "$BATS_TEST_DIRNAME/expected_1"
}

@test "Testing $TEST_NAME - header [case 1]" {
    check_output_parser "$BATS_TEST_DIRNAME/parser.h"
}

@test "Testing $TEST_NAME - source [case 1]" {
    check_output_parser "$BATS_TEST_DIRNAME/parser.c"
}

@test "Testing $TEST_NAME - generation [case 2]" {
    PACKCC_OPTS=("--lines")
    test_generate
    make_expected "$BATS_TEST_DIRNAME/expected_2"
}

@test "Testing $TEST_NAME - header [case 2]" {
    check_output_parser "$BATS_TEST_DIRNAME/parser.h"
}

@test "Testing $TEST_NAME - source [case 2]" {
    check_output_parser "$BATS_TEST_DIRNAME/parser.c"
}
