test_generate () {
    (cd "$1" && "$PACKCC" -o "parser" "input.peg")
}

test_compile() {
    local dir="$1"
    "${CC:-cc}" -I "$dir" "main.c" -o "$dir/parser"
}

run_for_input() {
    run "$dir/parser" < "$1"
    [ "$status" -eq 0 ]
    diff -uN "${1/input/expected}" --label "${1/input/expected}" <(echo "$output") --label "output"
}

test_run () {
    local dir="$1"

    for input in "$dir"/input*.txt; do
        run_for_input "$input"
    done
}
