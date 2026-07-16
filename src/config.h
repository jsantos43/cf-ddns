#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
  char api_token[200];
  char zone_id[100];
  char record_id[100];
  char record_name[100];
} ddns_config;

int read_config_from_file(ddns_config *config);

#endif