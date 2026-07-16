#include "config.h"
#include <stdio.h>

int read_config_from_file(ddns_config *config) {
  FILE *config_file = fopen("config.txt", "r");

  if (config_file == NULL) {
    return 1;
  }

  fscanf(config_file, "%s", config->api_token);
  fscanf(config_file, "%s", config->zone_id);
  fscanf(config_file, "%s", config->record_id);
  fscanf(config_file, "%s", config->record_name);

  fclose(config_file);

  return 0;
}