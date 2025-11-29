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

#######################################
#        monitor test cases            #
#######################################

# 41 - default interval works (no error)
run_test "./wirefish --monitor" 0 ""

# 42 - invalid interval string
run_test "./wirefish --monitor --interval abc" 1 "" "Error: Invalid interval value"

# 43 - zero interval
run_test "./wirefish --monitor --interval 0" 1 "" "Error: Interval must be positive"

# 44 - negative interval
run_test "./wirefish --monitor --interval -50" 1 "" "Error: Interval must be positive"

# 45 - nonexistent interface (monitor_run prints error)
run_test "./wirefish --monitor --iface defNotReal --interval 200" 1 "" "Error"

# 46 - auto-detect interface (success)
run_test "./wirefish --monitor --interval 200" 0 ""

# 47 - interface lo exists
run_test "./wirefish --monitor --iface lo --interval 200" 0 ""

# 48 - json + csv both true error
run_test "./wirefish --monitor --interval 200 --json --csv" 1 "" "Error: Cannot use both"

# 49 - unknown argument
run_test "./wirefish --monitor --interval 200 --what" 1 "" "Error: Unknown argument"

# 50 - monitor + scan together not allowed
run_test "./wirefish --monitor --scan --interval 200" 1 "" "Error: Only one mode"

# 51 - running with no mode at all
run_test "./wirefish" 1 "" "Error: Must specify one mode"

# 52 - using --ttl in monitor is allowed, just ignored
run_test "./wirefish --monitor --ttl 1-5 --interval 200" 0 ""

# 53 - weird iface name monitor_run error
run_test "./wirefish --monitor --iface ###weird### --interval 200" 1 "" "Error"

# 54 - user gives --csv alone 
run_test "./wirefish --monitor --interval 200 --csv" 0 ""

# 55 - long iface name likely invalid expect error
run_test "./wirefish --monitor --iface aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa --interval 200" 1 "" "Error"

# 56 - big but safe interval that still returns 
run_test "./wirefish --monitor --interval 2000" 0 "IFACE" ""

# 57 - missing interval value 
run_test "./wirefish --monitor --interval" 1 "" "Error: --interval requires a number (milliseconds)"

# 58 - missing iface value
run_test "./wirefish --monitor --iface" 1 "" "Error: --iface requires an interface name"

# 59 - invalid ttl format
run_test "./wirefish --monitor --ttl hello --interval 200" 1 "" "Range must be in format"

# 60 - json output is allowed (starts with '{')
run_test "./wirefish --monitor --interval 200 --json" 0 "{" ""

# 61 - iface given + default interval, prints header
run_test "./wirefish --monitor --iface lo" 0 "IFACE" ""

# 62 - fake iface again 
run_test "./wirefish --monitor --iface fakelo --interval 200" 1 "" "Error"


#######################################
# tracer tests 
#######################################

# 63 - forgot to put target with trace
run_test "./wirefish --trace" 1 "" "Error: --target required for trace mode"

# 64 - ttl backwards so it should error
run_test "./wirefish --trace --target 8.8.8.8 --ttl 10-1" 1 "" "Range start"

# 65 - letters inside ttl so that’s invalid
run_test "./wirefish --trace --target 8.8.8.8 --ttl abc-10" 1 "" "Invalid number"

# 66 - ttl is outside allowed minimum
run_test "./wirefish --trace --target 8.8.8.8 --ttl 0-5" 1 "" "TTL values must be in range"

# 67 - ttl max too high
run_test "./wirefish --trace --target 8.8.8.8 --ttl 1-300" 1 "" "TTL values must be in range"

# 68 - tracer can't resolve this name
run_test "./wirefish --trace --target noSuchHostXYZ123 --ttl 1-4" 1 "" "Failed to resolve"

# 69 - tracer always fails at raw socket in CI
run_test "./wirefish --trace --target 8.8.8.8 --ttl 1-3" 1 "" "requires root privileges"

# 70 - csv still dies the same because socket fail comes first
run_test "./wirefish --trace --target 8.8.8.8 --ttl 1-3 --csv" 1 "" "requires root privileges"

# 71 - json also ends the same way
run_test "./wirefish --trace --target 8.8.8.8 --ttl 1-3 --json" 1 "" "requires root privileges"

#######################################
# icmp 
#######################################

# 72 - tracer builds the icmp packet before failing at raw socket
run_test "./wirefish --trace --target 1.1.1.1 --ttl 5-5" 1 "" "requires root privileges"

#######################################
# net 
#######################################

# 73 - dns failure inside scan mode
run_test "./wirefish --scan --target FakeHost555 --ports 1-1" 1 "" "Failed to resolve"

# 74 - scan on loopback with a tiny range 
run_test "./wirefish --scan --target 127.0.0.1 --ports 1-1" 0 "PORT  STATE" ""

# 75 - unroutable ip gives filtered
run_test "./wirefish --scan --target 203.0.113.1 --ports 9-9" 0 "filtered" ""

# 76 - raw socket fail checked again
run_test "./wirefish --trace --target 1.1.1.1 --ttl 1-1" 1 "" "requires root privileges"

#######################################
# config tests (kind of)
#######################################

# 77 - json + csv not allowed on trace either
run_test "./wirefish --trace --target 8.8.8.8 --json --csv" 1 "" "Cannot use both"

# 78 - weird characters in port range
run_test "./wirefish --scan --target 127.0.0.1 --ports 5-XYZ" 1 "" "Invalid number"

# 79 - iface name is messed up so monitor errors
run_test "./wirefish --monitor --iface !!?!weird!! --interval 100" 1 "" "Interface"

#######################################
# format testing
#######################################

# 80 - monitor in csv mode should print header
run_test "./wirefish --monitor --interval 200 --csv" 0 "iface,rx_bytes" ""

# 81 - scan csv header check
run_test "./wirefish --scan --target 127.0.0.1 --ports 2-2 --csv" 0 "port,state,latency_ms" ""

# 82 - scan json check
run_test "./wirefish --scan --target 127.0.0.1 --ports 2-2 --json" 0 "\"results\"" ""

# 83 - monitor json output starts with {
run_test "./wirefish --monitor --interval 200 --json" 0 "{" ""

# 84 - scan table format just needs the main header
run_test "./wirefish --scan --target 127.0.0.1 --ports 3-3" 0 "PORT  STATE" ""

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
