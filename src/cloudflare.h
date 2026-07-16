#ifndef CLOUDFLARE_H
#define CLOUDFLARE_H
#include "config.h"

int read_record_ip(ddns_config config, char *ip, int ip_length);

#endif