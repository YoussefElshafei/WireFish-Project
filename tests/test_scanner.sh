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
#      test cases    #
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

#######################################
# more cli stuff
#######################################

# 85 - unknown flag here
run_test "./wirefish --wat" 1 "" "Unknown argument"

# 86 - mixing json and csv with scan is illegal
run_test "./wirefish --scan --target 127.0.0.1 --ports 1-2 --json --csv" 1 "" "Cannot use both"

# 87 - same thing but for monitor
run_test "./wirefish --monitor --interval 200 --json --csv" 1 "" "Cannot use both"

# 88 - giving ttl to scan doesn't matter, still should work
run_test "./wirefish --scan --target 127.0.0.1 --ports 1-1 --ttl 5-10" 0 "PORT  STATE" ""

# 89 - reversed ports again
run_test "./wirefish --scan --target 127.0.0.1 --ports 10-1" 1 "" "cannot be greater"

# 90 - forgot value after ttl
run_test "./wirefish --trace --target 8.8.8.8 --ttl" 1 "" "requires a range"

# 91 - forgot value after ports
run_test "./wirefish --scan --target 127.0.0.1 --ports" 1 "" "Error: --ports requires"

# 92 - forgot iface name
run_test "./wirefish --monitor --iface" 1 "" "requires an interface name"

#######################################
# more net 
#######################################

# 93 - scan on max port on loopback 
run_test "./wirefish --scan --target 127.0.0.1 --ports 65535-65535" 0 "65535" ""

# 94 - small range to activate table code paths
run_test "./wirefish --scan --target 127.0.0.1 --ports 1-3" 0 "PORT  STATE" ""

# 95 - monitor with auto interface detection
run_test "./wirefish --monitor --interval 200" 0 ""

#######################################
# more tracer fail checks 
#######################################

# 96 - big ttl range, still fails right away at socket
run_test "./wirefish --trace --target 8.8.8.8 --ttl 1-30" 1 "" "requires root"

# 97 - csv + big ttl range also fails right away
run_test "./wirefish --trace --target 8.8.8.8 --ttl 1-30 --csv" 1 "" "requires root"

# 98 - json + big ttl range same thing
run_test "./wirefish --trace --target 8.8.8.8 --ttl 1-30 --json" 1 "" "requires root"

#######################################
# config
#######################################

# 99 - scan without target
run_test "./wirefish --scan --ports 1-3" 1 "" "target required"

# 100 - trace without target
run_test "./wirefish --trace --ttl 1-3" 1 "" "target required"

#######################################
# trying app.c full
#######################################

# 101 - invalid iface should make monitor_run fail (tests run_monitor error path)
run_test "./wirefish --monitor --iface defielynotrealXYZ --interval 100" 1 "" "monitor mode failed"

# 102 - monitor json formatting work
run_test "./wirefish --monitor --interval 100 --json" 0 "{" ""

# 103 - monitor csv formatting work
run_test "./wirefish --monitor --interval 100 --csv" 0 "iface,rx_bytes" ""

# 104 - scan json formatting
run_test "./wirefish --scan --target 127.0.0.1 --ports 9-9 --json" 0 "\"results\"" ""

# 105 - scan csv formatting branch
run_test "./wirefish --scan --target 127.0.0.1 --ports 9-9 --csv" 0 "port,state,latency_ms" ""

# 106 - bad target so scanner_run fails
run_test "./wirefish --scan --target invalidhostnameZZZ --ports 10-10" 1 "" "Scan failed"

# 107 - normal scan hitting table format again
run_test "./wirefish --scan --target 127.0.0.1 --ports 5-7" 0 "PORT  STATE" ""

# 108 - monitor auto-detect again (tests iface==NULL path)
run_test "./wirefish --monitor --interval 50" 0 "" ""

# 109 - tracer always fails raw socket so this tests run_trace fail path
run_test "./wirefish --trace --target 8.8.8.8 --ttl 1-1" 1 "" "Traceroute failed"

# 110 - tracer csv also still hits same early error path
run_test "./wirefish --trace --target 8.8.8.8 --ttl 1-1 --csv" 1 "" "Traceroute failed"

# 111 - tracer json same idea  error)
run_test "./wirefish --trace --target 8.8.8.8 --ttl 1-1 --json" 1 "" "Traceroute failed"

# 112 - another normal scan single-port
run_test "./wirefish --scan --target 127.0.0.1 --ports 100-100" 0 "PORT  STATE" ""

# 113 - weird iface name so monitor should error
run_test "./wirefish --monitor --iface '!!!!' --interval 100" 1 "" "Error"

# 114 - bigger scan range to trigger realloc inside ScanTable
run_test "./wirefish --scan --target 127.0.0.1 --ports 1-3000" 0 "PORT  STATE" ""

# 115 - filtered scan using unroutable IP
run_test "./wirefish --scan --target 203.0.113.1 --ports 2-2" 0 "filtered" ""

# 116 - monitor with valid interface lo, should be fine
run_test "./wirefish --monitor --iface lo --interval 100" 0 "IFACE" ""

# 117 - another small scan range to hit formatting path again
run_test "./wirefish --scan --target 127.0.0.1 --ports 50-52" 0 "PORT  STATE" ""

#######################################
# cli tests
#######################################

# 118 - invalid characters inside range before dash (tests parse_range endptr!=dash)
run_test "./wirefish --scan --target 127.0.0.1 --ports 1x-10" 1 "" "Invalid characters in range"

# 119 - invalid characters after dash (tests parse_range invalid number after dash)
run_test "./wirefish --scan --target 127.0.0.1 --ports 10-9x" 1 "" "Invalid characters at end of range"

# 120 - range without dash at all (parse_range dash==NULL)
run_test "./wirefish --scan --target 127.0.0.1 --ports 123" 1 "" "Range must be in format"

# 121 - missing target after --target 
run_test "./wirefish --scan --target" 1 "" "Error: --target requires"

# 122 - missing ports value 
run_test "./wirefish --scan --target 127.0.0.1 --ports" 1 "" "Error: --ports requires"

# 123 - missing ttl value 
run_test "./wirefish --trace --target 8.8.8.8 --ttl" 1 "" "Error: --ttl requires"

# 124 - missing iface value (tests --iface requires)
run_test "./wirefish --monitor --iface" 1 "" "Error: --iface requires"

# 125 - missing interval value (tests --interval requires)
run_test "./wirefish --monitor --interval" 1 "" "Error: --interval requires"

# 126 - invalid number before dash 
run_test "./wirefish --scan --target 127.0.0.1 --ports x5-10" 1 "" "Invalid number before"

# 127 - invalid number aftrer dash 
run_test "./wirefish --scan --target 127.0.0.1 --ports 5-x10" 1 "" "Invalid number after"

# 128 - interval invalid letters inside argument tests strtol != number
run_test "./wirefish --monitor --interval xyz" 1 "" "Invalid interval value"

# 129 - interval < 0 inside final monitor block 
run_test "./wirefish --monitor --interval -5" 1 "" "Interval must be positive"

# 130 - interval zero also hits final validate block
run_test "./wirefish --monitor --interval 0" 1 "" "Interval must be positive"

# 131 - duplicate mode error specifically triggered on monitor 
run_test "./wirefish --scan --monitor --target 127.0.0.1 --ports 1-1" 1 "" "Only one mode"

# 132 - help printing 
run_test "./wirefish --help" 0 "Usage: wirefish" ""

# 133 - test unknown argument error path 
run_test "./wirefish --unknownFlag" 1 "" "Unknown argument"

# 134 - scan with botg json & csv 
run_test "./wirefish --scan --target 127.0.0.1 --ports 5-5 --json --csv" 1 "" "Cannot use both"

# 135 - trace with BOTH json & csv 
run_test "./wirefish --trace --target 8.8.8.8 --ttl 1-5 --json --csv" 1 "" "Cannot use both"

# 136 - monitor with BOTH json & csv
run_test "./wirefish --monitor --interval 100 --json --csv" 1 "" "Cannot use both"

# 137 - scan port outside valid range error inside SCAN block
run_test "./wirefish --scan --target 127.0.0.1 --ports 0-10" 1 "" "Ports must be in range"

# 138 - > 65535
run_test "./wirefish --scan --target 127.0.0.1 --ports 1-99999" 1 "" "Ports must be in range"

# 139 - trace TTL < minimum  hit TTL range error
run_test "./wirefish --trace --target 8.8.8.8 --ttl 0-5" 1 "" "TTL values must be in range"

# 140 - trace TTL > maximum  error path again
run_test "./wirefish --trace --target 8.8.8.8 --ttl 1-500" 1 "" "TTL values must be in range"

#######################################
# fmt.c 
#######################################

# 141 - scan table format default output, hits fmt_scan_table_table
run_test "./wirefish --scan --target 127.0.0.1 --ports 1-3" 0 "PORT  STATE" ""

# 142 - scan csv format hits fmt_scan_table_csv
run_test "./wirefish --scan --target 127.0.0.1 --ports 2-2 --csv" 0 "port,state,latency_ms" ""

# 143 - scan json format hits fmt_scan_table_json
run_test "./wirefish --scan --target 127.0.0.1 --ports 3-3 --json" 0 "\"results\"" ""

# 144 - scan filtered state hits port_state_str PORT_FILTERED path
run_test "./wirefish --scan --target 203.0.113.1 --ports 4-4" 0 "filtered" ""

#######################################
# extra tests to get coverage  for cli fmt monitor 
#######################################

# 145 - --help should print usage and exit successfully
run_test "./wirefish --help" 0 "Usage: wirefish" ""

# 146 - --help should mention monitor mode
run_test "./wirefish --help" 0 "--monitor" ""

# 147 - ports range: junk in the middle 
run_test "./wirefish --scan --target 127.0.0.1 --ports 1x-5" 1 "" "Invalid characters in range"

# 148 - ports range: junk at the end 
run_test "./wirefish --scan --target 127.0.0.1 --ports 1-5x" 1 "" "Invalid characters at end of range"

# 149 - monitor with auto-detected interface 
run_test "./wirefish --monitor --interval 200" 0 "IFACE" ""

# 150 - monitor on loopback
run_test "./wirefish --monitor --iface lo --interval 200" 0 "IFACE" ""

# 151 - monitor JSON output basic check
run_test "./wirefish --monitor --iface lo --interval 200 --json" 0 "\"type\":\"monitor\"" ""

# 152 - monitor CSV output basic check
run_test "./wirefish --monitor --iface lo --interval 200 --csv" 0 "iface,rx_bytes,tx_bytes" ""

# 153 - monitor invalid interface should report not found
run_test "./wirefish --monitor --iface notreal123 --interval 200" 1 "" "not found in /proc/net/dev"

# 154 - monitor interval = 0 should error
run_test "./wirefish --monitor --iface lo --interval 0" 1 "" "Interval must be positive"

# 155 - monitor negative interval should also error
run_test "./wirefish --monitor --iface lo --interval -5" 1 "" "Interval must be positive"

# 156 - cannot use more than one mode at once 
run_test "./wirefish --scan --monitor --target 127.0.0.1 --ports 80-80" 1 "" "Only one mode (--scan, --trace, --monitor) allowed"

# 157 - unknown argument should be rejected
run_test "./wirefish --scan --target 127.0.0.1 --ports 80-80 --weirdflag" 1 "" "Error: Unknown argument '--weirdflag'"

# 158 - scan mode cannot use both JSON and CSV at the same time
run_test "./wirefish --scan --target 127.0.0.1 --ports 80-80 --json --csv" 1 "" "Error: Cannot use both --json and --csv"

# 159 - trace mode missing target must error
run_test "./wirefish --trace" 1 "" "Error: --target required for trace mode"

# 160 - TTL values below allowed range should error
run_test "./wirefish --trace --target 8.8.8.8 --ttl 0-5" 1 "" "TTL values must be in range"

# 161 - ports below allowed range should error
run_test "./wirefish --scan --target 127.0.0.1 --ports 0-10" 1 "" "Ports must be in range"

# 162 -  ports above allowed range should also error
run_test "./wirefish --scan --target 127.0.0.1 --ports 1-70000" 1 "" "Ports must be in range"

# 163 - TTL range: junk in the middle 
run_test "./wirefish --trace --target 8.8.8.8 --ttl 1x-5" 1 "" "Invalid characters in range"

# 164 - TTL range: junk at the end (
run_test "./wirefish --trace --target 8.8.8.8 --ttl 1-5x" 1 "" "Invalid characters at end of range"

# 165 - TTL range: invalid number before '-' hits that specific error
run_test "./wirefish --trace --target 8.8.8.8 --ttl ab-5" 1 "" "Invalid number before '-' in range"

# 166 - TTL range: invalid number after '-' hits that specific error
run_test "./wirefish --trace --target 8.8.8.8 --ttl 1-ab" 1 "" "Invalid number after '-' in range"

# 167 - --help anywhere in the args should still just show help and exit
run_test "./wirefish --scan --target 127.0.0.1 --ports 80-80 --help" 0 "Usage: wirefish" ""

# 168 - JSON without picking a mode should fail with 'Must specify one mode'
run_test "./wirefish --json" 1 "" "Must specify one mode"

# 169 - CSV without picking a mode should also fail the same way
run_test "./wirefish --csv" 1 "" "Must specify one mode"

# 170 - running with no arguments at all must complain about missing mode
run_test "./wirefish" 1 "" "Must specify one mode"

# 171 - trace with default TTL 
run_test "./wirefish --trace --target 8.8.8.8" 1 "" "requires root privileges"

# 172 - trace and json with default TTL
run_test "./wirefish --trace --target 8.8.8.8 --json" 1 "" "requires root privileges"

# 173 - trace and csv with default TTL
run_test "./wirefish --trace --target 8.8.8.8 --csv" 1 "" "requires root privileges"

# 174 - monitor: iface given + ttl + json
run_test "./wirefish --monitor --iface lo --ttl 1-5 --interval 200 --json" 0 "{" ""

# 175 - monitor: iface given + tt + csv
run_test "./wirefish --monitor --iface lo --ttl 1-5 --interval 200 --csv" 0 "iface,rx_bytes,tx_bytes" ""

# 176 - scan: give ttl (ignored) + json, on loopback
run_test "./wirefish --scan --target $TARGET --ports 5-7 --ttl 2-5 --json" 0 "\"results\"" ""

# 177 - scan: give ttl (ignored) + csv, on loopback
run_test "./wirefish --scan --target $TARGET --ports 5-7 --ttl 2-5 --csv" 0 "port,state,latency_ms" ""

# 178 - monitor: tiny interval so it collects quickly
run_test "./wirefish --monitor --interval 1" 0 "IFACE" ""

# 179 - monitor: tiny interval + json
run_test "./wirefish --monitor --interval 1 --json" 0 "{" ""

# 180 - monitor: tiny interval + csv
run_test "./wirefish --monitor --interval 1 --csv" 0 "iface,rx_bytes" ""

# 181 - scan: json flag before mode (checks we don't depend on arg order)
run_test "./wirefish --json --scan --target $TARGET --ports 1-3" 0 "\"results\"" ""

# 182 - scan: csv flag before mode
run_test "./wirefish --csv --scan --target $TARGET --ports 1-3" 0 "port,state,latency_ms" ""

# 183 - monitor: json flag before mode
run_test "./wirefish --json --monitor --interval 200" 0 "{" ""

# 184 - monitor: csv flag before mode
run_test "./wirefish --csv --monitor --interval 200" 0 "iface,rx_bytes" ""

# 185 - scan: mode at end to make sure parsing still behaves
run_test "./wirefish --target $TARGET --ports 1-3 --scan" 0 "PORT  STATE" ""

# 186 - monitor: mode at end
run_test "./wirefish --interval 200 --monitor" 0 "IFACE" ""

# 187 - trace: mode at end (still hits raw-socket fail)
run_test "./wirefish --target 8.8.8.8 --ttl 1-3 --trace" 1 "" "requires root privileges"

# 188 - scan: long but safe loopback range to re-hit realloc in ScanTable
run_test "./wirefish --scan --target $TARGET --ports 1-1500" 0 "PORT  STATE" ""

# 189 - monitor: weird but valid-ish iface name trimmed to buffer size
run_test "./wirefish --monitor --iface looooooooooooooooooooooooooooo --interval 200" 1 "" "Interface" ""

# 190 - help still works even with stray json
run_test "./wirefish --help --json" 0 "Usage: wirefish" ""

# 191 - 1 on loopback is always closed
run_test "./wirefish --scan --target 127.0.0.1 --ports 1-1" 0 "closed" "-"

# 192 - scan TABLE: open port should show numeric latency
run_test "./wirefish --scan --target google.com --ports 80-80" 0 "open" "LATENCY"

# 193 - scan JSON: closed port should have  null
run_test "./wirefish --scan --target 127.0.0.1 --ports 1-1 --json" 0 "\"latency_ms\":null" ""

# 194 - monitor TABLE format check  - verify header
run_test "./wirefish --monitor --interval 200 --iface lo" 0 "RX_BYTES" ""

# 195 - monitor: iface empty string should error (--iface \"\")
run_test "./wirefish --monitor --interval 200 --iface \"\"" 1 "" "Error: --iface requires"

# 196 - cli_parse: target empty string should error (--target \"\")
run_test "./wirefish --scan --target \"\" --ports 80-80" 1 "" "Error: --target requires"

# 197 - monitor: both --tls and --ports given should still succeed
run_test "./wirefish --monitor --interval 200 --ttl 1-5 --ports 1-3" 0 "" ""

# 198 - scan: give extremely long hostname (buffer truncation test)
run_test "./wirefish --scan --target aaaaaaaaaaaaaaa.com --ports 1-1" 1 "" "Failed to resolve"

# 199 - tracer table formatting path
run_test "./wirefish --trace --target 8.8.8.8 --ttl 1-1" 1 "" "Traceroute failed"



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
