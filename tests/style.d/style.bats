#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

test_style() {
    if ! command -v "uncrustify" &> /dev/null; then
        skip "uncrustify is not installed"
    else
        run uncrustify -q -c "$TESTDIR/uncrustify.cfg" -f "$1"
        #~ echo "STATUS: $status, OUT:$output"
        [ "$status" -eq 0 ]
        diff -uN "$1" --label "$1" <(echo "$output") --label "formatted"
    fi
}

@test "Testing style.d - sources" {
    for file in "$ROOTDIR"/*/*.c; do
        test_style "$file"
    done
}

@test "Testing style.d - generated" {
    test_generate "style.d" "calc.peg"
    test_style "style.d/parser.h"
    test_style "style.d/parser.c"
}
