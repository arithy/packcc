#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

@test "Testing $TEST_NAME - generation" {
    test_generate "$ROOTDIR/examples/ast-calc.v3.peg"
}

@test "Testing $TEST_NAME - compilation" {
    $CC $CFLAGS -I "$BATS_TEST_DIRNAME" "$BATS_TEST_DIRNAME/parser.c" -o "$BATS_TEST_DIRNAME/parser" "$@"
}

@test "Testing $TEST_NAME - run" {
    run_for_input "$BATS_TEST_DIRNAME/input.txt"
}
