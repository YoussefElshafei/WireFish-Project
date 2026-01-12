# 🐟 Wirefish – Network Diagnostics & Monitoring Toolkit

![Release](https://img.shields.io/github/v/release/YoussefElshafei/WireFish-Project)
![License](https://img.shields.io/github/license/YoussefElshafei/WireFish-Project)
![Stars](https://img.shields.io/github/stars/YoussefElshafei/WireFish-Project)

**Wirefish** is a lightweight, command-line network diagnostics suite designed for education in **systems programming** and networking. It unifies host scanning, traceroute, and interface bandwidth monitoring into a single tool, demonstrating key concepts like **raw sockets**, **ICMP construction**, and live interface monitoring.

---

## ✨ Features

### ✔ Host Scanner
* Performs basic host reachability scans using **ICMP echo requests**.
* Collects per-host statistics, including **Round-Trip Time (RTT)** and success/fail counts.

### ✔ Traceroute (ICMP-based)
* Implements a simplified traceroute using **raw ICMP sockets**.
* Constructs ICMP Echo packets manually (including checksums and headers). 

[Image of the ICMP packet structure]

* Uses **increasing TTL values** to discover and map intermediate hops.
* Resolves hostnames when possible, printing hop number, IP address, hostname, and RTT.

### ✔ Interface Bandwidth Monitor
* Polls the Linux-specific **`/proc/net/dev`** file to read interface RX (receive) and TX (transmit) byte counters.
* Computes **instantaneous RX/TX bitrate (bps)** and rolling averages.
* **Clean Output Separation:** `monitor.c` gathers data and calculates rates; `fmt.c` handles all formatting and printing.
* Supports user-defined interface, sample interval, and duration.

### ✔ Unified CLI Front-End
All functionality is accessed via a single binary:
* `./wirefish --scan --subnet 192.168.1.0/24`
* `./wirefish --trace <host>`
* `./wirefish --monitor --iface eth0 --interval 100`

---

## 🚀 Key Technical Concepts

Wirefish is an educational project focusing on the following low-level networking and system access techniques:

### Raw Sockets & ICMP Construction
The **scanner** and **traceroute** modules use **Raw Sockets** to gain low-level protocol access. This requires manual construction of the **ICMP packet**, including the header (Type, Code, ID, Seq) and calculating the correct **16-bit Internet checksum** within `icmp.c`.

### Reading Interface Stats (Rate Calculation)
The monitor module computes the network speed using the counters read from `/proc/net/dev`.
The rate calculation is:
**rate**<sub>bps</sub> = (Δbytes × 8) / Δt<sub>sec</sub>

### Modular C Design
The project structure enforces clean separation of duties, such as the clear split between the core logic (`monitor.c`, `tracer.c`) and the output formatting (`fmt.c`), making the code easy to understand and maintain.

---

## ⚙️ Project Structure

The codebase is organized into logical, domain-specific directories:

| Directory | Purpose |
| :--- | :--- |
| `app/` | Main application dispatcher (routes CLI command to correct module) |
| `cli/` | Command-line argument parsing |
| `scanner/` | Host scanner logic |
| `tracer/` | Traceroute logic (`tracer.c` + `icmp.c`) |
| `monitor/` | Interface bandwidth monitor logic |
| `fmt/` | Output formatting (text, JSON, CSV) |
| `net/` | Generic socket utilities |
| `log/` | **Logging subsystem** with level-based filtering |

### 💬 Logging Subsystem (`log/`)
Provides `printf`-style logging with configurable severity: `LOG_DEBUG (0)`, `LOG_INFO (1)`, `LOG_WARN (2)`, `LOG_ERROR (3)`. All output is written to **stderr** with level tags (e.g., `[error]`).

---

## 💻 Command Summary

| Mode | Option | Description | Default |
| :--- | :--- | :--- | :--- |
| **Scanner** | `--scan --subnet (CIDR)` | Scan for hosts in a CIDR block | N/A (Required) |
| **Traceroute** | `--trace --target (host)` | Map route to a host/IP | N/A (Required) |
| **Traceroute** | `--ttl (start-max)` | TTL range to use | 1-30 |
| **Monitor** | `--monitor --iface (name)` | Network interface (e.g., `eth0`) | Auto-detect |
| **Monitor** | `--interval (ms)` | Sample interval in milliseconds | 100 |
| **Monitor** | `--duration (seconds)` | Total run time (0 = infinite) | 0 |
| **Output** | `--json` / `--csv` | Change output format | Text |
| **Other** | `--help` | Show usage message | N/A |

---

## 🛠 Build Instructions

### Requirements
* **GCC**
* **Make**
* **Linux Environment** (Mandatory for raw sockets and `/proc/net/dev` access)

### Build & Run
```bash
# Build the executable
make

# Example: Run traceroute
./wirefish --trace google.com

# Example: Run bandwidth monitor
./wirefish --monitor --iface eth0 --interval 100

```

## Limitations
- Linux only (requires `/proc/net/dev` and raw sockets)  
- Root privileges required for traceroute & scanning  
- Simplified error-handling as this is a teaching project  

## Authors
Shan Truong,
Aryan Verma,
Youssef E,
Youssef K,

McMaster University, 2025
