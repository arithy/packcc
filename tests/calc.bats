#!/usr/bin/env bats

load utils.bash

@test "generate calc" {
    simple_test src/examples/calc.peg
}
