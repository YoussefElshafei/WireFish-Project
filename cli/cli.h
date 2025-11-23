/*
 * File: cli.h
 * Summary: Command-line parsing (--scan, --trace, --monitor, flags)
 *
 * Responsibilities:
 *  - Parse argc and argv into a CommandLine struct
 *  - Validate args and provide help text
 *
 * Function prototypes:
 *  - int cli_parse(int argc, char *argv[], CommandLine *out)
 *  - void cli_print_help()
 * 
 * Aryan Verma, 400575438, McMaster University 
 */

#include <stdbool.h>


// These are the values used when the user doesn't specify certain options
#define DEFAULT_PORTS_FROM 1        // Start scanning at port 1
#define DEFAULT_PORTS_TO 1024       // Scan up to port 1024 (common ports)
#define DEFAULT_TTL_START 1         // Start traceroute with TTL=1 
#define DEFAULT_TTL_MAX 30          // Max 30 hops for traceroute 
#define DEFAULT_INTERVAL_MS 1000    // Sample every 1000ms = 1 second 

// These define the valid ranges for various parameters
// Ports: TCP/UDP ports range from 1 to 65535
// TTL: IP Time-To-Live field is 8 bits, so 1-255
#define MIN_PORT 1
#define MAX_PORT 65535
#define MIN_TTL 1
#define MAX_TTL 255



/*
 * Struct: CommandLine
 * 
 * This structure holds all the parsed command-line arguments in one place
 *
 * HELP and OUTPUT FORMAT:
 *   show_help - true if user typed --help (we show help and exit)
 *   json      - true if user wants JSON output format
 *   csv       - true if user wants CSV output format
 *
 * TARGET and INTERFACE:
 *   target    - The hostname or IP address to scan/trace (ex "google.com", "192.168.1.1")
 *               Size 256 is plenty for any valid hostname (max DNS name is 253 chars)
 *   iface     - Network interface name for monitoring (ex "eth0", "wlan0")
 *               Size 64 is standard for interface names (typically much shorter)
 *
 * PORT SCANNING:
 *   ports_from - Starting port number (ex 80)
 *   ports_to   - Ending port number (ex 443)
 *                Valid ports are 1-65535 (TCP/UDP port range)
 *
 * TRACEROUTE TTL:
 *   ttl_start  - Starting Time-To-Live value (usually 1)
 *   ttl_max    - Maximum TTL to try (usually 30)
 *                TTL determines how many "hops" a packet can make
 *
 * MONITORING:
 *   interval_ms - How often to sample network stats, in milliseconds
 *                 (e.g., 1000 = sample once per second)
 *
 * MODE:
 *   mode - Which operation mode the user selected (scan, trace, or monitor)
 *          We use an enum to make this type-safe and clear
 */

typedef struct {
  // Holds if user requested help
  bool show_help;

  // Specific output formats 
  bool json, csv;

  // Target and interface strings
  char target[256];
  char iface[64];

  // Port range for scanning
  int ports_from, ports_to;

  // Time-To-Live ranges 
  // Start is usually 1 and max is usually 30
  int ttl_start, ttl_max;

  // Sample rate or monitoring interval
  int interval_ms;

  // Which mode to use, only one can be active at a time
  enum {
    MODE_NONE=0, // No mode selected (should be error, since this state should not happen)
    MODE_SCAN,   // Port scan mode
    MODE_TRACE,  // Traceroute mode
    MODE_MONITOR // Network monitoring mode 
  } mode;

} CommandLine;


int cli_parse(int argc, char *argv[], CommandLine *out);
void cli_print_help();

