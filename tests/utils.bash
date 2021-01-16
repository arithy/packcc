report_lines ()
{
    local line

    if [[ ${#lines[@]} -gt 0 ]]; then
	echo "# "
    fi
    
    for line in "${lines[@]}"; do
	echo "# $line"
    done
} >&3

#
# simple_test input.peg
# 
simple_test ()
{
    local cmd

    local input=$1
    local base=$(basename "$input" .peg)
    local generated_base=tests/"$base".generated
    local expected_base=tests/"$base".expected

    # force inserting new line to the output
    echo "# " >&3
    for cmd in build/*/bin/*/packcc; do
	if ! [[ -x "$cmd" ]]; then
	    continue
	fi

	echo "# target executable: $cmd" >&3
	
	# Check exist status
	run "$cmd" -o "$generated_base" "$input"
	[ "$status" -eq 0 ]

	# Compare .c file
	run diff -uN "$expected_base".c "$generated_base".c
	report_lines	
	[ "$status" -eq 0 ]
	rm "$generated_base".c

	# Compare .h file
	run diff -uN "$expected_base".h "$generated_base".h
	report_lines
	[ "$status" -eq 0 ]
	rm "$generated_base".h
    done
    return 0
}
