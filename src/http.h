#ifndef HTTP_H
#define HTTP_H
#include <stddef.h>

struct response_buffer {
  char *data;
  size_t size;
};

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp);

#endif