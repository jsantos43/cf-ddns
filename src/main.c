/*
A script to update cloudflare record ip address based on my local network
Author: João Pedro Tomaz dos Santos
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "config.h"
#include "http.h"
#include "cloudflare.h"
#include "ipv4.h"
#include "error.h"

#define IP_LENGTH 46

int main(void) {
  ddns_config config;
  int err = read_config_from_file(&config);
  if (err != DDNS_OK) {
    fprintf(stderr, "Failed to read config: %s\n", ddns_error_str(err));
    return err;
  }

  curl_global_init(CURL_GLOBAL_DEFAULT);

  char record_ip[IP_LENGTH];
  err = read_record_ip(config, record_ip, IP_LENGTH);
  if (err != DDNS_OK) {
    fprintf(stderr, "Failed to read the Cloudflare record: %s\n", ddns_error_str(err));
    return err;
  }

  char public_ip[IP_LENGTH];
  err = read_public_ip(public_ip, IP_LENGTH);
  if (err != DDNS_OK) {
    fprintf(stderr, "Failed to get the public IP: %s\n", ddns_error_str(err));
    return err;
  }

  printf("CURRENT IP: %s\n", public_ip);
  printf("RECORD IP: %s\n", record_ip);

  if (strcmp(record_ip, public_ip) != 0) {
    printf("IPs addresses are not equal!\n");
    
    err = update_record_ip(config, public_ip);
    if (err == DDNS_OK) {
      printf("Record ip address updated!\n");
    } else {
      fprintf(stderr, "Failed to update the record: %s\n", ddns_error_str(err));
      return err;
    }
  } else {
    printf("IPs addresses are synchronized!\n");
  }

  curl_global_cleanup();

  printf("All done!\n");

  return 0;
}