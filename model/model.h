/*
 * File: model.h
 * Summary: Shared data models used by fmt/* and public APIs.
 *
 * Purpose:
 *  - Centralize the common structs so formatters can include a single header
 *  - Avoid circular includes between feature modules
 *
 * Contains:
 *  - typedefs mirrored from scanner.h (ScanResult, ScanTable)
 *  - typedefs mirrored from tracer.h  (Hop, TraceRoute)
 *  - typedefs mirrored from monitor.h (IfaceStats, MonitorSeries)
 *
 * Note:
 *  - Keep in sync with feature headers or include them conditionally.
 */
#ifndef MODEL_H
#define MODEL_H
/* Option 1: re-typedef here (dup types); Option 2: include feature headers */
#endif /* MODEL_H */
