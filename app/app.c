/*
 * File: app.c
 * Implements main control flow and error-to-exit-code mapping.
 *
 * Exit Codes (example policy):
 *  - 0 OK
 *  - 2 invalid arguments
 *  - 3 network resolution error
 *  - 4 permission error (raw sockets)
 *  - 5 runtime/IO error
 */
