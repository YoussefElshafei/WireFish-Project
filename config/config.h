/*
 * File: config.h
 * Summary: Validated runtime configuration shared by all modules.
 *
 * Responsibilities:
 *  - Convert parsed CLI into sanitized Config
 *  - Centralize defaults and cross-field validation
 *
 * Public API:
 *  - int config_from_cli(const CommandLine *cli, Config *cfg);
 *  - int config_validate(const Config *cfg);
 */
#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

typedef struct {
  char target[256];
  char iface[64];
  int ports_from, ports_to;
  int ttl_start, ttl_max;
  int interval_ms;
  bool output_json, output_csv;
} Config;

struct CommandLine;

int config_from_cli(const struct CommandLine *cli, Config *cfg);
int config_validate(const Config *cfg);

#endif /* CONFIG_H */
