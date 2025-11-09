/*
 * File: log.h
 * Summary: Tiny logging utility with severity levels.
 *
 * Public API:
 *  - void log_set_level(LogLevel lvl);
 *  - void log_debug/info/warn/error(const char *fmt, ...);
 */
#ifndef LOG_H
#define LOG_H

typedef enum { LOG_DEBUG=0, LOG_INFO, LOG_WARN, LOG_ERROR } LogLevel;

void log_set_level(LogLevel lvl);
void log_debug(const char *fmt, ...);
void log_info (const char *fmt, ...);
void log_warn (const char *fmt, ...);
void log_error(const char *fmt, ...);

#endif /* LOG_H */
