#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

@test "Testing unused_functions.d - generated" {
    for file in "$TESTDIR"/unused_functions.d/*.peg; do
        test_generate "$file"
        gcc -fsigned-char -Wall -Wextra -Wno-unused-parameter -Wno-overlength-strings -pedantic -Werror \
            -c unused_functions.d/parser.c -o unused_functions.d/parser.o
    done
}
