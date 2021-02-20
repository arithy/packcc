#!/usr/bin/env bats

load $TESTDIR/utils.sh

calc_compile() {
    "${CC:-cc}" calc.d/parser.c -o calc.d/parser
}

@test "Testing calc.d" {
    test_generate calc.d
    calc_compile
    test_run calc.d
}
