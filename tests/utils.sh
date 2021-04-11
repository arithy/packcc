test_generate () {
    (cd "$1" && "$PACKCC" -o "parser" "${2:-input.peg}")
}

test_compile() {
    local dir="$1"
    shift
    ${CC:-cc} -I "$dir" "main.c" -o "$dir/parser" "$@"
}

check_output() {
    diff --strip-trailing-cr -uN "${1/input/expected}" --label "${1/input/expected}" <(echo "$output") --label "output"
}

run_for_input() {
    run timeout 5s "$(dirname "$1")/parser" < "$1"
    check_output "$1"
}
