#!/usr/bin/env bash

build() {
    if [ -z "$PACKCC" ]; then
        export PACKCC="$BENCHDIR/packcc"
        echo "Building packcc..."
        ${CC:-cc -O2} -o "$PACKCC" $ROOTDIR/src/packcc.c
    fi
}

clean() {
    rm -f packcc parser* *.out
}

format() {
    TIME="$1"
    if [ $(($TIME / 1000000000)) -gt 10 ]; then
        echo "$(($TIME / 1000000000)) s"
    elif [ $(($TIME / 1000000)) -gt 10 ]; then
        echo "$(($TIME / 1000000)) ms"
    elif [ $(($TIME / 1000)) -gt 10 ]; then
        echo "$(($TIME / 1000)) us"
    else
        echo "$(($TIME)) ns"
    fi
}

measure() {
    COUNT="$1"
    shift
    START="$(date '+%s%N')"
    for ((i=0; i<$COUNT; i++)); do
        "$@"
    done
    END="$(date '+%s%N')"
    TIME=$(( $END - $START ))
}

benchmark() {
    KEY="$1"
    BENCHMARKS+=("$KEY")
    shift
    NAME="parser_$KEY"

    echo "Generating $KEY parser ($GEN_REPEATS times)..."
    "$PACKCC" --debug "$@" -o "$NAME" "$GRAMMAR" > "dump_$KEY.log"
    measure $GEN_REPEATS "$PACKCC" "$@" -o "$NAME" "$GRAMMAR"
    GENERATION["$KEY"]=$TIME
    echo "  Repeated $GEN_REPEATS times in $(format $TIME)"

    echo "Building $KEY parser..."
    measure $BUILD_REPEATS ${CC:-cc -O2} "$NAME".c -o "$NAME"
    BUILD["$KEY"]=$TIME
    echo "  Built $BUILD_REPEATS times in $(format $TIME)"

    echo "Running $KEY parser ($REPEATS times)..."
    "./$NAME" < "$INPUT" &> "$KEY.out"
    measure $REPEATS "./$NAME" < "$INPUT" &> /dev/null
    RESULTS["$KEY"]=$TIME
    echo "  Repeated $REPEATS times in $(format $TIME)"
}

print_results() {
    TAB=$'\t'
    FORMAT="%-12s %-15s %-15s %-15s %8s %8s\n"
    printf "\n$FORMAT" "Test" "Generation" "Build" "Run" "Rules" "Diff?"
    for KEY in "${BENCHMARKS[@]}"; do
        DIFF="$(diff "base.out" "$KEY.out" | grep -c '^[<>]' || true)"
        RELATIVE="$((100*RESULTS["$KEY"]/RESULTS[base]))"
        AVG="$((RESULTS["$KEY"]/$REPEATS))"
        GEN_AVG="$((GENERATION["$KEY"]/$GEN_REPEATS))"
        GEN_RELATIVE="$((100*GENERATION["$KEY"]/${GENERATION[base]}))"
        BUILD_AVG="$((BUILD["$KEY"]/$BUILD_REPEATS))"
        BUILD_RELATIVE="$((100*BUILD["$KEY"]/${BUILD[base]}))"
        RULES="$(grep -c '^Rule(' "dump_$KEY.log")"
        printf "$FORMAT" "$KEY" "$(format $GEN_AVG) ($GEN_RELATIVE%)" "$(format $BUILD_AVG) ($BUILD_RELATIVE%)" "$(format $AVG) ($RELATIVE%)" "$RULES" "$DIFF"
    done
}

main() {
    set -e

    export BENCHDIR="$(cd "$(dirname "$0")" && pwd)"
    export ROOTDIR="$BENCHDIR/../.."
    declare -a BENCHMARKS=()
    declare -A RESULTS=()
    declare -A GENERATION=()

    REPEATS="${REPEATS:-100}"
    GEN_REPEATS="${GEN_REPEATS:-100}"
    BUILD_REPEATS="${BUILD_REPEATS:-3}"
    GRAMMAR="${GRAMMAR:-kotlin.peg}"
    INPUT="${INPUT:-input.kt}"

    cd "$BENCHDIR"
    clean
    build

    benchmark base
    benchmark inline -O 1
    benchmark concat -O 2
    benchmark all -O 1 -O 2

    print_results
}

main "$@"
