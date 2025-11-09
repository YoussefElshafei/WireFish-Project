/*
 * File: app.h
 * Summary: Top-level orchestrator (CLI → Config → Module → Formatter).
 *
 * Public API:
 *  - int app_run(int argc, char **argv);
 *
 * Behavior:
 *  - Parses CLI, builds Config, dispatches to scan/trace/monitor
 *  - Selects output mode (table/CSV/JSON) and exit codes
 */
#ifndef APP_H
#define APP_H
int app_run(int argc, char **argv);
#endif /* APP_H */
