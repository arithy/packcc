#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

@test "Testing ast-calc.v3.d - generation" {
    test_generate "$ROOTDIR/examples/ast-calc.v3.peg"
}

@test "Testing ast-calc.v3.d - compilation" {
    ${CC:-cc} ast-calc.v3.d/parser.c -o ast-calc.v3.d/parser
}

@test "Testing ast-calc.v3.d - run" {
    run_for_input ast-calc.v3.d/input.txt
}
