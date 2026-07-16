#include "http.h"
#include <stdlib.h>
#include <string.h>

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t real_size = size * nmemb;
  struct response_buffer *resp = (struct response_buffer *)userp;

  char *ptr = realloc(resp->data, resp->size + real_size + 1);
  if (!ptr) return 0;
  resp->data = ptr; // update the pointer in case realloc moved the memory block

  // append the incoming contents to the end of the buffer
  memcpy(&(resp->data[resp->size]), contents, real_size);
  resp->size += real_size;
  resp->data[resp->size] = '\0'; // ensure a null-terminated string

  return real_size;
}