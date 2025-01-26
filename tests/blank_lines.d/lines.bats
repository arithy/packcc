#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

is_newline_terminated() {
    cat "$BATS_TEST_DIRNAME/$1" | tail -n1 | wc -l | grep 1
}

get_last_line() {
    cat "$BATS_TEST_DIRNAME/$1" | wc -l
    # NOTE: The input file must be terminated with a newline character.
}

extract_line() {
    sed -n "$1"p "$BATS_TEST_DIRNAME/$2"
}

count_blank_lines() {
    grep -c $'^[ \t\v\f]*$'
}

check_if_enclosed_by_blank_lines() {
    LINE=$(get_line "$1" "$2")
    if [ "$LINE" -gt 1 ]; then
        [ "$LINE" -ne 2 ]
        [ $(extract_line $((LINE - 1)) "$2" | count_blank_lines) -eq 1 ]
        [ $(extract_line $((LINE - 2)) "$2" | count_blank_lines) -eq 0 ]
    fi
    TAIL=$(get_last_line "$2")
    if [ "$LINE" -lt "$TAIL" ]; then
        [ "$LINE" -ne $((TAIL - 1)) ]
        [ $(extract_line $((LINE + 1)) "$2" | count_blank_lines) -eq 1 ]
        [ $(extract_line $((LINE + 2)) "$2" | count_blank_lines) -eq 0 ]
    fi
}

check_if_not_enclosed_by_blank_lines() {
    LINE=$(get_line "$1" "$2")
    if [ "$LINE" -gt 1 ]; then
        [ $(extract_line $((LINE - 1)) "$2" | count_blank_lines) -eq 0 ]
    fi
    TAIL=$(get_last_line "$2")
    if [ "$LINE" -lt "$TAIL" ]; then
        [ $(extract_line $((LINE + 1)) "$2" | count_blank_lines) -eq 0 ]
    fi
}

@test "Testing blank_lines.d - generation" {
    test_generate
}

@test "Testing blank_lines.d - header" {
    is_newline_terminated parser.h
    check_if_enclosed_by_blank_lines "HEADER 0" parser.h
    check_if_enclosed_by_blank_lines "HEADER 1" parser.h
    check_if_enclosed_by_blank_lines "COMMON 0" parser.h
    check_if_enclosed_by_blank_lines "COMMON 1" parser.h
    check_if_enclosed_by_blank_lines "EARLY HEADER 0" parser.h
    check_if_enclosed_by_blank_lines "EARLY HEADER 1" parser.h
    check_if_enclosed_by_blank_lines "EARLY COMMON 0" parser.h
    check_if_enclosed_by_blank_lines "EARLY COMMON 1" parser.h
}

@test "Testing blank_lines.d - source" {
    is_newline_terminated parser.c
    check_if_enclosed_by_blank_lines "SOURCE 0" parser.c
    check_if_enclosed_by_blank_lines "SOURCE 1" parser.c
    check_if_enclosed_by_blank_lines "COMMON 0" parser.c
    check_if_enclosed_by_blank_lines "COMMON 1" parser.c
    check_if_enclosed_by_blank_lines "EARLY SOURCE 0" parser.c
    check_if_enclosed_by_blank_lines "EARLY SOURCE 1" parser.c
    check_if_enclosed_by_blank_lines "EARLY COMMON 0" parser.c
    check_if_enclosed_by_blank_lines "EARLY COMMON 1" parser.c
    check_if_enclosed_by_blank_lines "FOOTER" parser.c
    check_if_not_enclosed_by_blank_lines "ACTION 0" parser.c
    check_if_not_enclosed_by_blank_lines "ACTION 1" parser.c
    check_if_not_enclosed_by_blank_lines "ACTION 2" parser.c
}
