#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

@test "Testing ast-calc.d - generation" {
    test_generate "$ROOTDIR/examples/ast-calc.peg"
}

@test "Testing ast-calc.d - compilation" {
    $CC $CFLAGS -I "$BATS_TEST_DIRNAME" "$BATS_TEST_DIRNAME/parser.c" -o "$BATS_TEST_DIRNAME/parser" "$@"
}

@test "Testing ast-calc.d - run" {
    run_for_input ast-calc.d/input.txt
}
