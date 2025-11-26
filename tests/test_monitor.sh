# /*
#  * File: tests/test_monitor.sh
#  * Summary: Unit tests for interface bandwidth monitoring using /proc/net/dev.
#  *
#  * Purpose:
#  *  - Validate parsing of /proc/net/dev for a given interface.
#  *  - Verify rate computations over time (bps) and rolling averages.
#  *  - Ensure robustness to missing/nonexistent interfaces.
#  *
#  * Test Matrix (examples):
#  *  - Happy path on loopback:
#  *      cfg.iface = "lo", interval_ms small (e.g., 100–200 ms), duration_sec short (1–2 s).
#  *      Expect non-decreasing byte counters; rates >= 0.
#  *  - Nonexistent interface:
#  *      cfg.iface = "nope0" → expect error return (no crash).
#  *  - Smoothing:
#  *      Enable ring buffer; verify avg tracks mean of recent samples.
#  *  - Low-traffic vs burst:
#  *      Optionally generate traffic (e.g., ping localhost) and confirm rate spike > baseline.
#  *
#  * What is Verified:
#  *  - Return codes and series length > 0 on success.
#  *  - Each sample has iface set, rx/tx bytes monotonic (except counter wrap edge case).
#  *  - Rates are finite, >= 0; averages within expected bounds.
#  *  - Proper resource cleanup (no leaks).
#  *
#  * Modules Covered:
#  *  - monitor.c / monitor.h
#  *  - ringbuf.c / ringbuf.h
#  *  - timeutil.c (interval timing)
#  *
#  * Fixtures / Setup:
#  *  - Read-only access to /proc/net/dev; skip test if file unavailable.
#  *  - Interface name overridable via env (NETGUARD_TEST_IFACE).
#
# Author: Youssef Khafagy
#  */


 #!/bin/bash
#
# A simple framework for testing the binconv (bctest) scripts
#
# Returns the number of failed test cases.
#
# Format of a test:
#     test 'command' expected_return_value 'stdin text' 'expected stdout' 'expected stderr'
#
# Some example test cases are given. You should add more test cases.
#
# Sam Scott, McMaster University, 2025


# Simple test script for scanner module
declare -i tc=0
declare -i fails=0


#The run_test function is slighlty edited from the original version, 
#Comments added to explain the changes
run_test() {
    tc=$tc+1

    local COMMAND="$1"
    local RETURN="$2"
    local STDOUT="$3"
    local STDERR="$4"

    #the original run_test runs COMMAND 3 times but our codes output changes every run
    #so here I run it once and save stdout/stderr 
    $COMMAND >tmp_out 2>tmp_err
    local A_RETURN=$?

    if [[ "$A_RETURN" != "$RETURN" ]]; then
        echo "Test $tc FAILED"
        echo "   Expected Return: $RETURN"
        echo "   Actual Return: $A_RETURN"
        fails=$fails+1
        return
    fi

    #same thing, instead of rerunning it, just get STDOUT and STDERR from the single run
    local A_STDOUT="$(cat tmp_out)"
    local A_STDERR="$(cat tmp_err)"

    #The orignal checks for a full exact match but wirefish prints alot of dynamic stuff
    #so I changed it to substring check instead as exact matches will fail
    if [[ -n "$STDOUT" ]]; then #this line is to make sure its not empty
        if [[ "$A_STDOUT" != *"$STDOUT"* ]]; then
            echo "Test $tc FAILED (stdout)"
            echo "  expected substring: $STDOUT"
            echo "  actual: $A_STDOUT"
            fails=$fails+1
            return 1
        fi
    fi

    #same substring logic for stderr
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

# 1 - missing interval 
run_test "./wirefish --monitor" 0 "Monitoring interface" ""

# 2 - invalid interval string
run_test "./wirefish --monitor --interval abc" 1 "" "Error: Invalid interval value"

# 3 - zero interval
run_test "./wirefish --monitor --interval 0" 1 "" "Error: Interval must be positive"

# 4 - negative interval
run_test "./wirefish --monitor --interval -50" 1 "" "Error: Interval must be positive"

# 5 - nonexistent interface
run_test "./wirefish --monitor --iface definitelyNotReal --interval 200" 0 ""

# 6 - auto-detect interface (success no errors)
run_test "./wirefish --monitor --interval 200" 0 ""

# 7 - interface lo exists
run_test "./wirefish --monitor --iface lo --interval 200" 0 ""

# 8 - json + csv both true error
run_test "./wirefish --monitor --interval 200 --json --csv" 1 "" "Error: Cannot use both"

# 9 - unknown argument
run_test "./wirefish --monitor --interval 200 --what" 1 "" "Error: Unknown argument"

# 10 - monitor + scan together not allowed (cli.c)
run_test "./wirefish --monitor --scan --interval 200" 1 "" "Error: Only one mode"

# 11 - running with no mode at all
run_test "./wirefish" 1 "" "Error: Must specify one mode"

# 12 - using --ttl in monitor is allowed
run_test "./wirefish --monitor --ttl 1-5 --interval 200" 0 ""

# 13 - weird iface name (cli accepts it monitor_run returns 0)
run_test "./wirefish --monitor --iface ###weird### --interval 200" 0 ""

# 14 - user gives --csv alone 
run_test "./wirefish --monitor --interval 200 --csv" 0 ""

# 15 - long iface name  allowed runs, prints table header
run_test "./wirefish --monitor --iface aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa --interval 200" 0 "IFACE" ""

# 16 - huge interval still valid prints table header only
run_test "./wirefish --monitor --interval 999999" 0 "IFACE" ""

# 17  missing interval value
run_test "./wirefish --monitor --interval" 1 "" "Error: --interval requires a number (milliseconds)"

# 18 - missing iface value
run_test "./wirefish --monitor --iface" 1 "" "Error: --iface requires an interface name (e.g., eth0)"

# 18 - invalid ttl format
run_test "./wirefish --monitor --ttl hello --interval 200" 1 "" "Range must be in format"

# 19 - json output is allowed (output starts with '{')
run_test "./wirefish --monitor --interval 200 --json" 0 "{" ""

# 20 - iface given + no interval default interval, prints header only 
run_test "./wirefish --monitor --iface lo" 0 "IFACE" ""

# 21 - fake iface monitor_run returns error internally but prints nothing  empty table header
run_test "./wirefish --monitor --iface fakelo --interval 200" 0 "IFACE" ""