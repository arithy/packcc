#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

@test "Testing unused_rule_elimination.d - generation" {
    run "$PACKCC" --debug -o "unused_rule_elimination.d/parser" "unused_rule_elimination.d/input.peg" 2>&1
    check_output "unused_rule_elimination.d/expected.txt"
}
