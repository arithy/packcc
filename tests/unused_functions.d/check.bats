#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

@test "Testing unused_functions.d - generated" {
    for file in "$BATS_TEST_DIRNAME"/*.peg; do
        test_generate "$file"
        $CC $CFLAGS -I "$BATS_TEST_DIRNAME" -c "$BATS_TEST_DIRNAME/parser.c" -o "$BATS_TEST_DIRNAME/parser.o"
    done
}
