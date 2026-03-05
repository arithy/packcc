#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

@test "Testing ast-calc.v3.d - generation" {
    test_generate "$ROOTDIR/examples/ast-calc.v3.peg"
}

@test "Testing ast-calc.v3.d - compilation" {
    $CC $CFLAGS -I "$BATS_TEST_DIRNAME" "$BATS_TEST_DIRNAME/parser.c" -o "$BATS_TEST_DIRNAME/parser" "$@"
}

@test "Testing ast-calc.v3.d - run" {
    run_for_input ast-calc.v3.d/input.txt
}
