/*
A script to update cloudflare ipv4 based on my local network
Author: João Pedro Tomaz dos Santos
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h"

#define CF_API_BASE_URL "https://api.cloudflare.com/client/v4"
#define IP_LENGTH 46

typedef struct {
  char api_token[200];
  char zone_id[200];
  char record_id[200];
  char record_name[50];
} ddns_config;

struct response_buffer {
  char *data;
  size_t size;
};


int parse_current_ip(const char *response, char *ipv4) {
  cJSON *root = cJSON_Parse(response);
  if (root == NULL) {
    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr != NULL) {
      fprintf(stderr, "Erro ao parsear JSON perto de: %s\n", error_ptr);
    }

    return -1;
  }

  // Checa se a API retornou sucesso
  cJSON *success = cJSON_GetObjectItemCaseSensitive(root, "success");
  if (!cJSON_IsTrue(success)) {
    fprintf(stderr, "API retornou erro\n");
    cJSON_Delete(root);

    // return -1;
  }

      cJSON *errors = cJSON_GetObjectItemCaseSensitive(root, "errors");

    if (!cJSON_IsArray(errors)) {
        fprintf(stderr, "Resposta de erro em formato inesperado\n");
    }

    cJSON *error_item = NULL;
    cJSON_ArrayForEach(error_item, errors) {
        cJSON *code = cJSON_GetObjectItemCaseSensitive(error_item, "code");
        cJSON *message = cJSON_GetObjectItemCaseSensitive(error_item, "message");

        if (cJSON_IsNumber(code) && cJSON_IsString(message)) {
            fprintf(stderr, "Erro Cloudflare [%d]: %s\n",
                    code->valueint, message->valuestring);
        }
    }

  // Navega: root -> result -> content
  cJSON *result = cJSON_GetObjectItemCaseSensitive(root, "result");
  cJSON *content = cJSON_GetObjectItemCaseSensitive(result, "content");

  if (!cJSON_IsString(content) || content->valuestring == NULL) {
      fprintf(stderr, "Campo 'content' não encontrado ou inválido\n");
      cJSON_Delete(root);
      return -1;
  }

  snprintf(ipv4, IP_LENGTH, "%s", content->valuestring);

  cJSON_Delete(root); // libera a árvore inteira
  return 0;
}

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t real_size = size * nmemb;
  struct response_buffer *resp = (struct response_buffer *)userp;

  char *ptr = realloc(resp->data, resp->size + real_size + 1);
  if (!ptr) {
    fprintf(stderr, "Erro: sem memória suficiente\n");
    return 0;
  }
  resp->data = ptr; // Atualiza o ponteiro se o meu realloc mudar o bloco de memória
  
  // Copia o conteúdo que está chegando do contents para o final do meu buffer
  memcpy(&(resp->data[resp->size]), contents, real_size);
  resp->size += real_size;
  resp->data[resp->size] = '\0'; // garante string null-terminated

  return real_size;
}

ddns_config read_config_from_file() {
  FILE *config_file = fopen("config.txt", "r");

  if (config_file == NULL) {
    fprintf(stderr, "Error to read ddns config file\n");
    // return;
  }

  ddns_config config;

  fscanf(config_file, "%s", config.api_token);
  fscanf(config_file, "%s", config.zone_id);
  fscanf(config_file, "%s", config.record_id);
  fscanf(config_file, "%s", config.record_name);

  fclose(config_file);

  return config;
}

int read_public_ipv4(char *ipv4) {
  CURL *curl;
  CURLcode res;
  int result = 0;
  struct response_buffer resp = { .data = malloc(1), .size = 0 };

  curl = curl_easy_init();

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.ipify.org");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); // timeout in seconds

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      result = 1;
    } else {
      snprintf(ipv4, IP_LENGTH, "%s", resp.data);
    }

    curl_easy_cleanup(curl);
  }

  free(resp.data);

  return result;
}

int read_dns_record(ddns_config config, char *current_ipv4) {
  CURL *curl;
  CURLcode res;
  int result = 0;
  struct response_buffer resp = { .data = malloc(1), .size = 0 };

  char url[456];
  snprintf(
    url, 
    sizeof(url),    
    "%s/zones/%s/dns_records/%s",
    CF_API_BASE_URL,
    config.zone_id,
    config.record_id
  );

  printf("%s %s\n", config.api_token, config.zone_id);

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
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); // timeout in seconds

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      result = 1;
    } else {
      // printf("Resposta bruta: %s\n", resp.data);
      parse_current_ip(resp.data, current_ipv4);
    }
  }

  free(resp.data);
  return result;
}

int update_dns_record(const char *zone_id, const char *record_id,
                       const char *api_token, const char *new_ip) {
    CURL *curl;
    CURLcode res;
    struct response_buffer resp = { .data = malloc(1), .size = 0 };
    int success = 0;

    char url[256];
    snprintf(url, sizeof(url),
             "https://api.cloudflare.com/client/v4/zones/%s/dns_records/%s",
             zone_id, record_id);

    char auth_header[512];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", api_token);

    char json_body[256];
    snprintf(json_body, sizeof(json_body),
             "{\"type\":\"A\",\"content\":\"%s\"}", new_ip);

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
            fprintf(stderr, "Erro na requisição: %s\n", curl_easy_strerror(res));
        } else {
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            if (http_code == 200) {
                printf("DNS atualizado com sucesso: %s\n", resp.data);
                success = 1;
            } else {
                fprintf(stderr, "Falha (HTTP %ld): %s\n", http_code, resp.data);
            }
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    free(resp.data);
    return success;
}

int main(void) {
  ddns_config config = read_config_from_file();

  curl_global_init(CURL_GLOBAL_DEFAULT);

  char ipv4[IP_LENGTH];
  char current_ip[IP_LENGTH];

  if(read_public_ipv4(ipv4) != 0) {
    fprintf(stderr, "Error to request ipv4\n");
    return 1;
  }

  if(read_dns_record(config, current_ip) != 0) {
    fprintf(stderr, "Error to read cloudflare record ip\n");
    return 1;
  }

  if (strcmp(current_ip, ipv4) != 0) {
    printf("IPs addresses not equal!");
    // make update
  }

  printf("IP: %s", ipv4);
  printf("CURRENT IP: %s", current_ip);

  curl_global_cleanup();

  return 0;
}