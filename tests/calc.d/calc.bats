#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

@test "Testing calc.d - generation" {
    test_generate "$ROOTDIR/examples/calc.peg"
}

@test "Testing calc.d - compilation" {
    $CC $CFLAGS -I "$BATS_TEST_DIRNAME" "$BATS_TEST_DIRNAME/parser.c" -o "$BATS_TEST_DIRNAME/parser" "$@"
}

@test "Testing calc.d - run" {
    run_for_input calc.d/input.txt
}
