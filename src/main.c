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

#define IP_LENGTH 46

int main(void) {
  ddns_config config;
  if (read_config_from_file(&config) != 0) {
    fprintf(stderr, "Error to read config file\n");
    return 1;
  }
  
  curl_global_init(CURL_GLOBAL_DEFAULT);

  char record_ip[IP_LENGTH];
  int read_record_result = read_record_ip(config, record_ip, IP_LENGTH);
  if (read_record_result != 0) {
    fprintf(stderr, "Error to read cloudflare record ip\n");
    return read_record_result;
  }

  char public_ip[IP_LENGTH];
  if (read_public_ip(public_ip, IP_LENGTH) != 0) {
    fprintf(stderr, "Error to read public ip via a request\n");
    return 4;
  }

  printf("CURRENT IP: %s\n", public_ip);
  printf("RECORD IP: %s\n", record_ip);

  if (strcmp(record_ip, public_ip) != 0) {
    printf("IPs addresses are not equal!\n");
    
    int update_record_result = update_record_ip(config, public_ip);
    if (update_record_result == 0) {
      printf("Record ip address updated!\n");
    } else {
      printf("Failed to update record ip address!\n");
      return update_record_result;
    }
  } else {
    printf("IPs addresses are synchronized!\n");
  }

  curl_global_cleanup();

  printf("All done!\n");

  return 0;
}