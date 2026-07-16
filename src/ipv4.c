#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

int read_public_ip(char *public_ip, int ip_length) {
  CURL *curl;
  CURLcode res;
  int result = 0;
  struct response_buffer resp = { .data = malloc(1), .size = 0 };

  curl = curl_easy_init();

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.ipify.org");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      result = 1;
    } else {
      snprintf(public_ip, ip_length, "%s", resp.data);
    }

    curl_easy_cleanup(curl);
  }

  free(resp.data);

  return result;
}