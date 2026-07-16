#include "http.h"
#include <stdlib.h>
#include <string.h>

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t real_size = size * nmemb;
  struct response_buffer *resp = (struct response_buffer *)userp;

  char *ptr = realloc(resp->data, resp->size + real_size + 1);
  if (!ptr) return 0;
  resp->data = ptr; // Atualiza o ponteiro se o meu realloc mudar o bloco de memória
  
  // Copia o conteúdo que está chegando do contents para o final do meu buffer
  memcpy(&(resp->data[resp->size]), contents, real_size);
  resp->size += real_size;
  resp->data[resp->size] = '\0'; // garante string null-terminated

  return real_size;
}