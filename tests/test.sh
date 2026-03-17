#!/usr/bin/env bash

generate_bats() {
    skip_all=""
    if [ -f "$1/input.skip.peg" ]; then
        skip_all=$'skip\n'
    fi
    cat <<EOF
#!/usr/bin/env bats

load "\$TESTDIR/utils.sh"

@test "Testing $1 - generation" {
    ${skip_all}test_generate
}

@test "Testing $1 - compilation" {
    ${skip_all}test_compile
}
EOF
    for input in "$1"/input*.txt; do
        skip="$skip_all"
        suffix="$(basename "${input/input-/}" .txt)"
        if [[ "$suffix" =~ .skip ]]; then
            skip=true
            suffix="${suffix/.skip/}"
        fi
        if [ "$suffix" = "input" ]; then
            suffix=""
        else
            suffix=" [$suffix]"
        fi
        echo "@test \"Testing $1 - run$suffix\" {"
        [ "$skip" ] && echo "    skip"
        echo "    run_for_input \"$input\""
        echo "}"
    done
}

build() {
    if [ -z "$PACKCC" ]; then
        export PACKCC="$TESTDIR/packcc$EXE"
        if [ -v WINDIR ]; then
            ${CC:-cl} $CFLAGS -o "$PACKCC" "$ROOTDIR/src/packcc.c"
        else
            ${CC:-gcc} $CFLAGS --coverage -O0 -o "$PACKCC" "$ROOTDIR/src/packcc.c"
        fi
    fi
}

clean() {
    rm -f packcc{$EXE,.gcda,.gcno,.c.gcov} *.d/test.bats *.d/parser{$EXE,.c,.h}
}

main() {
    set -e

    export TESTDIR="$(cd "$(dirname "$0")" && pwd)"
    export ROOTDIR="$TESTDIR/.."

    if [ -v WINDIR ]; then
        export CC=cl
        export CFLAGS="-W4 -WX -wd4100 -wd4456"
        export EXE=.exe
    else
        export CC=gcc
        export CFLAGS="-std=c90 -g -fsigned-char -Wall -Wextra -Wno-unused-parameter -Wno-overlength-strings -pedantic -Werror -fsanitize=undefined,address -fno-sanitize-recover=all -fno-omit-frame-pointer"
        export EXE=
    fi

    cd "$TESTDIR"
    clean
    build

    for DIR in *.d; do
        # Do not generate test file if the directory already contains some
        ls "$DIR"/*.bats &> /dev/null && continue
        generate_bats "$DIR" > "$DIR/test.bats"
    done

    bats "$@" ./*.d

    if [ -f "packcc.gcda" ]; then
        echo "$(gcov packcc | grep "Lines executed") (see $TESTDIR/packcc.c.gcov for details)"
    fi
}

main "$@"
