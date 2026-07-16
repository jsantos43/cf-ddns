#ifndef ERROR_H
#define ERROR_H

typedef enum {
  DDNS_OK          = 0,   // success
  DDNS_ERR_CONFIG  = 1,   // could not read/open config.txt
  DDNS_ERR_NETWORK = 2,   // curl did not complete the request (no network, timeout...)
  DDNS_ERR_API     = 3,   // Cloudflare returned an error (success:false / HTTP != 200)
  DDNS_ERR_PARSE   = 4,   // response is not valid JSON or an expected field is missing
  DDNS_ERR_UNKNOWN = 99,  // any unclassified failure
} ddns_error;

// readable message for an error code (handy in main).
static inline const char *ddns_error_str(int err) {
  switch (err) {
    case DDNS_OK:          return "success";
    case DDNS_ERR_CONFIG:  return "failed to read config.txt";
    case DDNS_ERR_NETWORK: return "network failure on the request";
    case DDNS_ERR_API:     return "Cloudflare returned an error (check token/zone/record)";
    case DDNS_ERR_PARSE:   return "unexpected response (invalid JSON or missing field)";
    default:               return "unknown error";
  }
}

#endif
