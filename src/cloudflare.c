#include <stdlib.h>
#include <curl/curl.h>
#include "config.h"
#include "http.h"
#include "json.h"
#include "cloudflare.h"
#include "error.h"

#define CF_API_BASE_URL "https://api.cloudflare.com/client/v4"

int read_record_ip(ddns_config config, char *ip, int ip_length) {
  CURL *curl;
  CURLcode res;
  int result = DDNS_ERR_UNKNOWN;
  struct response_buffer resp = { .data = malloc(1), .size = 0 };

  char url[512];
  snprintf(
    url, 
    sizeof(url),    
    "%s/zones/%s/dns_records/%s",
    CF_API_BASE_URL,
    config.zone_id,
    config.record_id
  );

  char auth_header[512];
  snprintf(
    auth_header, 
    sizeof(auth_header), 
    "Authorization: Bearer %s", 
    config.api_token
  );

  curl = curl_easy_init();

  if (curl) {
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      result = DDNS_ERR_NETWORK;
    } else {
      result = parse_record_ip(resp.data, ip, ip_length);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  }

  free(resp.data);

  return result;
}

int update_record_ip(ddns_config config, const char *ip) {
  CURL *curl;
  CURLcode res;
  int result = DDNS_ERR_UNKNOWN;
  struct response_buffer resp = { .data = malloc(1), .size = 0 };

  char url[512];
  snprintf(
    url, 
    sizeof(url),
    "%s/zones/%s/dns_records/%s",
    CF_API_BASE_URL,
    config.zone_id, 
    config.record_id
  );

  char auth_header[512];
  snprintf(
    auth_header, 
    sizeof(auth_header), 
    "Authorization: Bearer %s", 
    config.api_token
  );

  char json_body[256];
  snprintf(
    json_body, 
    sizeof(json_body),
    "{\"type\":\"A\",\"content\":\"%s\"}", 
    ip
  );

  curl = curl_easy_init();
  if (curl) {
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_body);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      result = DDNS_ERR_NETWORK;
    } else {
      long http_code = 0;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

      result = (http_code == 200) ? DDNS_OK : DDNS_ERR_API;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  }

  free(resp.data);
  
  return result;
}