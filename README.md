<<<<<<< HEAD

=======
# Wirefish – Network Diagnostics & Monitoring Toolkit

Wirefish is a lightweight command-line network diagnostics suite designed to provide scanning, traceroute, and interface bandwidth monitoring in a single unified tool. It is intended as an educational project demonstrating systems programming, raw sockets, ICMP construction, live interface monitoring, and modular C design.

## Features

### ✔ Host Scanner
- Performs basic host reachability scans.
- Uses ICMP echo requests to detect active hosts.
- Collects per-host statistics (RTT, success/fail counts).

### ✔ Traceroute (ICMP-based)
- Implements a simplified traceroute using raw ICMP sockets.
- Constructs ICMP Echo packets manually (checksum + headers).
- Uses increasing TTL values to discover intermediate hops.
- Resolves hostnames when possible.
- Prints hop number, IP address, hostname, and RTT.

### ✔ Interface Bandwidth Monitor
- Polls `/proc/net/dev` to read interface RX/TX counters.
- Computes instantaneous RX/TX bitrate (bps) and rolling averages.
- Fully decoupled from formatting: `monitor.c` only gathers data; `fmt.c` prints results.
- Handles Ctrl+C clean exit.
- Supports user-defined interface, interval, and duration.

### ✔ Unified CLI Front-End
All functionality is accessed via a single binary:

./wirefish --scan  
./wirefish --trace <host>  
./wirefish --monitor --iface eth0 --interval 100

## Project Structure
app/        – main dispatcher (CLI → correct module)  
cli/        – command-line argument parser  
scanner/    – host scanner (ICMP reachability)  
tracer/     – traceroute (tracer.c + icmp.c)  
monitor/    – interface bandwidth monitor  
fmt/        – output formatting for scanner/tracer/monitor  
net/        – generic socket utilities  
model/      – shared data models and structs  
timeutil/   – time utilities (ms_now, ms_sleep, timestamp formatting)

## Build Instructions

### Requirements
- GCC  
- Linux environment (raw sockets + /proc/net/dev)  
- Make  

### Build
make

### Run Examples
./wirefish --trace google.com  
./wirefish --monitor --iface eth0 --interval 100  
./wirefish --scan --subnet 192.168.1.0/24  

## Key Technical Concepts

### Raw Sockets
Used for traceroute and scanning to manually build ICMP packets, providing low-level protocol access.

### ICMP Packet Construction
`icmp.c` manually constructs:
- ICMP header (type, code, id, seq, checksum)  
- Payload  
- Correct 16-bit Internet checksum  

### Reading Interface Stats
The monitor module reads `/proc/net/dev` every interval:
- RX bytes  
- TX bytes  

Rates are computed using:
rate_bps = (delta_bytes * 8) / delta_time_sec

### Clean Output Separation
- `monitor.c` → calculates stats  
- `fmt.c` → prints formatted tables  

## Command Summary

### Traceroute
--trace <hostname or IP>

### Monitor
--monitor  
--iface <name>         (eth0, wlan0, etc.)  
--interval <ms>  
--duration <seconds>   (0 = infinite)

### Scanner
--scan --subnet <CIDR>

## Makefile Notes
Compiles and links all modules:

gcc -o wirefish \
    app/main.c app/app.c cli/cli.c \
    scanner/scanner.c tracer/tracer.c tracer/icmp.c \
    monitor/monitor.c fmt/fmt.c net/net.c \
    timeutil/timeutil.c

## Limitations
- Linux only (requires `/proc/net/dev` and raw sockets)  
- Root privileges required for traceroute & scanning  
- Simplified error-handling as this is a teaching project  

## Authors
Team 25 – DDOS Project
Shan Truong,
Aryan Verma,
Youssef E,
Youssef K,
McMaster University, 2025
>>>>>>> integration-shan-plus-aryan-plus-youssefE
