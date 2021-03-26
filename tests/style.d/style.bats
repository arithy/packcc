#!/usr/bin/env bats

@test "Testing style.d" {
    if command -v "clang-format" &> /dev/null; then
        run clang-format --style=file "$ROOTDIR/src/packcc.c"
        [ "$status" -eq 0 ]
        diff -uN "$ROOTDIR/src/packcc.c" --label "src/packcc.c" <(echo "$output") --label "formatted"
    else
        skip "clang-format is not installed"
    fi
}
