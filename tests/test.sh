#!/usr/bin/env bash

generate_bats() {
cat > "$1/test.bats" <<EOF
#!/usr/bin/env bats

load $TESTDIR/utils.sh

@test "Testing $1" {
    test_generate "$1"
    test_compile "$1"
    test_run "$1"
}
EOF
}

build() {
    if [ -z "$PACKCC" ]; then
        export PACKCC="$TESTDIR/packcc"
        "${CC:-cc}" -o "$PACKCC" $ROOTDIR/src/packcc.c
    fi
}

clean() {
    rm -f packcc *.d/test.bats *.d/parser{,.c,.h}
}

main() {
    set -e

    export TESTDIR="$(cd "$(dirname "$0")" && pwd)"
    export ROOTDIR="$TESTDIR/.."

    cd "$TESTDIR"
    clean
    build

    for DIR in *.d; do
        # Do not generate test file if the directory already contains some
        ls "$DIR"/*.bats &> /dev/null && continue
        generate_bats "$DIR"
    done

    bats "$@" ./*.d
}

main "$@"
