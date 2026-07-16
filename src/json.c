#include "json.h"
#include <stdio.h>
#include "cJSON.h"

int parse_record_ip(const char *response, char *ip, int ip_length) {
  cJSON *root = cJSON_Parse(response);

  // Error to parse
  if (root == NULL) {
    return 3;
  }

  // Api returned an error
  cJSON *success = cJSON_GetObjectItemCaseSensitive(root, "success");
  if (!cJSON_IsTrue(success)) {
    cJSON_Delete(root);

    return 4;
  }

  // Go to: root -> result -> content
  cJSON *result = cJSON_GetObjectItemCaseSensitive(root, "result");
  cJSON *content = cJSON_GetObjectItemCaseSensitive(result, "content");

  // Invalid or not found content field
  if (!cJSON_IsString(content) || content->valuestring == NULL) {
    cJSON_Delete(root);

    return 5;
  }

  snprintf(ip, ip_length, "%s", content->valuestring);

  cJSON_Delete(root);
  return 0;
}