#!/bin/bash
#
# File: tests/test_scanner.sh
# Summary: Unit tests for the TCP port-scanning module.  
# Author: Youssef Khafagy
#

# Simple test script for scanner module
declare -i tc=0
declare -i fails=0

# Use localhost for faster testing
TARGET="127.0.0.1"

run_test() {
    tc=$tc+1

    local COMMAND="$1"
    local RETURN="$2"
    local STDOUT="$3"
    local STDERR="$4"

    # Run command with 5-second timeout to prevent hanging
    timeout 5s $COMMAND >tmp_out 2>tmp_err
    local A_RETURN=$?

    if [[ "$A_RETURN" != "$RETURN" ]]; then
        echo "Test $tc FAILED"
        echo "   Expected Return: $RETURN"
        echo "   Actual Return: $A_RETURN"
        fails=$fails+1
        return
    fi

    local A_STDOUT="$(cat tmp_out)"
    local A_STDERR="$(cat tmp_err)"

    if [[ -n "$STDOUT" ]]; then
        if [[ "$A_STDOUT" != *"$STDOUT"* ]]; then
            echo "Test $tc FAILED (stdout)"
            echo "  expected substring: $STDOUT"
            echo "  actual: $A_STDOUT"
            fails=$fails+1
            return 1
        fi
    fi

    if [[ -n "$STDERR" ]]; then
        if [[ "$A_STDERR" != *"$STDERR"* ]]; then
            echo "Test $tc FAILED (stderr)"
            echo "  expected substring: $STDERR"
            echo "  actual: $A_STDERR"
            fails=$fails+1
            return
        fi
    fi

    echo "Test $tc passed"
}

#######################################
#      test cases (first 20 only)     #
#######################################

# 1 - simple scan on one known port
run_test "./wirefish --scan --target $TARGET --ports 80-80" 0 "PORT  STATE" ""

# 2 - multi-port scan
run_test "./wirefish --scan --target $TARGET --ports 80-81" 0 "PORT  STATE" ""

# 3 - CSV output should start with the CSV header
run_test "./wirefish --scan --target $TARGET --ports 80-81 --csv" 0 "port,state,latency_ms" ""

# 4 - JSON output must contain the "results" key
run_test "./wirefish --scan --target $TARGET --ports 80-81 --json" 0 "\"results\"" ""

# 5 - missing target
run_test "./wirefish --scan --ports 1-3" 1 "" "Error: --target required for scan mode"

# 6 - reversed port range
run_test "./wirefish --scan --target $TARGET --ports 10-1" 1 "" "Range start (10) cannot be greater than end (1)"

# 7 - port 0 not allowed
run_test "./wirefish --scan --target $TARGET --ports 0-5" 1 "" "Ports must be in range"

# 8 - port >65535
run_test "./wirefish --scan --target $TARGET --ports 1-70000" 1 "" "Ports must be in range"

# 9 - unresolvable host
run_test "./wirefish --scan --target nohost123456 --ports 1-3" 1 "" "Error: Failed to resolve target"

# 10 - unroutable IP (reduced to single port for speed)
run_test "./wirefish --scan --target 10.255.255.1 --ports 80-80" 0 "filtered" ""

# 11 - highest valid port
run_test "./wirefish --scan --target $TARGET --ports 65535-65535" 0 "65535" ""

# 12 - two modes specified error
run_test "./wirefish --scan --trace --target $TARGET --ports 80-81" 1 "" "Only one mode"

# 13 - json + csv not allowed
run_test "./wirefish --scan --target $TARGET --ports 80-81 --json --csv" 1 "" "Cannot use both"

# 14 - missing mode entirely
run_test "./wirefish --target $TARGET --ports 80-82" 1 "" "Must specify one mode"

# 15 - unknown argument
run_test "./wirefish --scan --target $TARGET --ports 80-82 --xyz" 1 "" "Error: Unknown argument '--xyz'"

# 16 - missing value after --ports
run_test "./wirefish --scan --target $TARGET --ports" 1 "" "Error: --ports requires"

# 17 - invalid characters in port range
run_test "./wirefish --scan --target $TARGET --ports abc-80" 1 "" "Invalid number"

# 18 - bigger port range (reduced from 1-100 to 1-20 for speed)
run_test "./wirefish --scan --target $TARGET --ports 1-20" 0 "PORT  STATE" ""

# 19 - CSV output for single port
run_test "./wirefish --scan --target $TARGET --ports 443-443 --csv" 0 "port,state,latency_ms" ""

# 20 - JSON output for single port
run_test "./wirefish --scan --target $TARGET --ports 443-443 --json" 0 "\"results\"" ""

# 21 - open port on an external site
run_test "./wirefish --scan --target google.com --ports 80-80" 0 "open" ""

# 22 - filtered port using an unroutable IP
run_test "./wirefish --scan --target 10.255.255.1 --ports 80-80" 0 "filtered" ""

# 23 - another target that should fail DNS resolution
run_test "./wirefish --scan --target notbadbadbad12345 --ports 20-22" 1 "" "Failed to resolve target"

# 24 - larger range to make the scan table grow
run_test "./wirefish --scan --target $TARGET --ports 1-2000" 0 "PORT  STATE" ""

# 25 - even bigger range to trigger more reallocations
run_test "./wirefish --scan --target $TARGET --ports 1-5000" 0 "PORT  STATE" ""

# 26 - very large range that still succeeds
run_test "./wirefish --scan --target $TARGET --ports 1-20000" 0 "PORT  STATE" ""

# 27 - filtered ports over a small range
run_test "./wirefish --scan --target 10.255.255.1 --ports 1-3" 0 "filtered" ""

# 28 - scan using default ports (no --ports given)
run_test "./wirefish --scan --target $TARGET" 0 "PORT  STATE" ""

# 29 - larger local range with JSON output
run_test "./wirefish --scan --target $TARGET --ports 1-50 --json" 0 "\"results\"" ""

# 30 - larger local range with CSV output
run_test "./wirefish --scan --target $TARGET --ports 1-50 --csv" 0 "port,state,latency_ms" ""

# 31 - filtered result on unroutable IP (single port so timeout is safe)
run_test "./wirefish --scan --target 10.255.255.1 --ports 1-1" 0 "filtered" ""

# 32 - another DNS failure test
run_test "./wirefish --scan --target doesnt_exist_12345 --ports 10-12" 1 "" "Failed to resolve target"

# 33 - mix of open and filtered ports on google (small range)
run_test "./wirefish --scan --target google.com --ports 80-82" 0 "open" ""

# 34 - scan single open port on google with JSON
run_test "./wirefish --scan --target google.com --ports 443-443 --json" 0 "\"results\"" ""

# 35 - scan single open port on google with CSV
run_test "./wirefish --scan --target google.com --ports 443-443 --csv" 0 "port,state,latency_ms" ""

# 36 - mid-size range on loopback (all filtered or closed, we just check table header)
run_test "./wirefish --scan --target $TARGET --ports 1-200" 0 "PORT  STATE" ""

# 37 - port at the top of the range on loopback
run_test "./wirefish --scan --target $TARGET --ports 65535-65535" 0 "65535" ""

# 38 - another unroutable IP
run_test "./wirefish --scan --target 203.0.113.1 --ports 1-3" 0 "filtered" ""

# 39 - valid hostname with a port that should be filtered
run_test "./wirefish --scan --target example.com --ports 81-81" 0 "filtered" ""

# 40 - mid-range on loopback with JSON
run_test "./wirefish --scan --target $TARGET --ports 30-60 --json" 0 "\"results\"" ""

# Cleanup
rm -f tmp_out tmp_err

# Print summary
echo "================================"
echo "Total tests: $tc"
echo "Failed tests: $fails"
echo "Passed tests: $((tc - fails))"
echo "================================"

# Exit with the number of failures
exit $fails
