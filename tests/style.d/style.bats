#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

check_uncrustify_version() {
    version="$(uncrustify --version)"
    major="$(echo "$version" | cut -d. -f1 | grep -oE '[0-9]+$')"
    minor="$(echo "$version" | cut -d. -f2)"
    [ "$major" -gt 0 ] || { [ "$major" -eq 0 ] && [ "$minor" -ge 72 ]; }
}

test_style() {
    if ! command -v "uncrustify" &> /dev/null; then
        skip "uncrustify is not installed"
    elif ! check_uncrustify_version &> /dev/null; then
        skip "uncrustify is too old (minimal required version is 0.72.0)"
    else
        run uncrustify -q -c "$TESTDIR/uncrustify.cfg" -f "$1"
        [ "$status" -eq 0 ]
        diff --strip-trailing-cr -uN "$1" --label "$1" <(echo "$output") --label "formatted"
    fi
}

@test "Testing $TEST_NAME - sources" {
    for file in "$ROOTDIR"/*/*.c; do
        test_style "$file"
    done
}

@test "Testing $TEST_NAME - generated [tests]" {
    for file in "$ROOTDIR"/tests/*.d/*.peg; do
        dir="$(dirname "$file")"
        if [ \
            "$dir" = "$ROOTDIR/tests/style.d" -o \
            "$dir" = "$ROOTDIR/tests/import.d" -o \
            "$dir" = "$ROOTDIR/tests/import_version_check.d" -o \
            "$dir" = "$ROOTDIR/tests/packcc_version_check.d" -o \
            "$dir" = "$ROOTDIR/tests/code_indentation.d" -o \
            "$dir" = "$ROOTDIR/tests/code_line_continuation.d" -o \
            "$dir" = "$ROOTDIR/tests/substitution.d" -o \
            "$dir" = "$ROOTDIR/tests/option_substitution.d" -o \
            "$dir" = "$ROOTDIR/tests/version_substitution.d" -o \
            "$dir" = "$ROOTDIR/tests/invalid_identifier_rvar.d" -o \
            "$dir" = "$ROOTDIR/tests/invalid_identifier_mvar.d" -o \
            "$dir" = "$ROOTDIR/tests/issue_78.d" \
        ]; then
            continue
        fi
        test_generate "$file"
        test_style "$BATS_TEST_DIRNAME/parser.h"
        test_style "$BATS_TEST_DIRNAME/parser.c"
    done
}

@test "Testing $TEST_NAME - generated [examples]" {
    for file in "$ROOTDIR"/examples/*.peg; do
        test_generate "$file"
        test_style "$BATS_TEST_DIRNAME/parser.h"
        test_style "$BATS_TEST_DIRNAME/parser.c"
    done
}
