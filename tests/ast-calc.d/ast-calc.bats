#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

@test "Testing ast-calc.d - generation" {
    test_generate "$ROOTDIR/examples/ast-calc.peg"
}

@test "Testing ast-calc.d - compilation" {
    ${CC:-cc} ast-calc.d/parser.c -o ast-calc.d/parser
}

@test "Testing ast-calc.d - run" {
    run_for_input ast-calc.d/input.txt
}
