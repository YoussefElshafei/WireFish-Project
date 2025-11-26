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
TARGET="127.0. 0.1"

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
#           test cases                #
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

# 21 - using localhost IP
run_test "./wirefish --scan --target 127.0.0.1 --ports 80-81" 0 "PORT  STATE" ""

# 22 - scanning localhost
run_test "./wirefish --scan --target 127.0.0.1 --ports 1-3" 0 "PORT  STATE" ""

# 23 - lowest allowed port number
run_test "./wirefish --scan --target $TARGET --ports 1-1" 0 "PORT  STATE" ""

# 24 - highest allowed port number
run_test "./wirefish --scan --target $TARGET --ports 65535-65535" 0 "65535" ""

# 25 - TEST-NET-1 unroutable (reduced to single port)
run_test "./wirefish --scan --target 192.0.2.1 --ports 80-80" 0 "filtered" ""

# 26 - scanning localhost (ports 1–3 are closed)
run_test "./wirefish --scan --target 127.0.0.1 --ports 1-3" 0 "closed" ""

# 27 - larger range to trigger realloc (reduced from 1-200 to 1-50)
run_test "./wirefish --scan --target $TARGET --ports 1-50" 0 "PORT  STATE" ""

# 28 - valid scan with unknown flag at end
run_test "./wirefish --scan --target $TARGET --ports 80-82 --badflag" 1 "" "Unknown argument"

# 29 - JSON flag with invalid flag
run_test "./wirefish --scan --target $TARGET --ports 80-82 --json --nope" 1 "" "Unknown argument"

# 30 - invalid port range
run_test "./wirefish --scan --target $TARGET --ports abc-80" 1 "" "Invalid number"

# 31 - missing left-hand number in range
run_test "./wirefish --scan --target $TARGET --ports -123" 1 "" "Error: Invalid characters in range '-123'"

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
