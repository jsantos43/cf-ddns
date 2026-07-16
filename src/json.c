#include "json.h"
#include "error.h"
#include <stdio.h>
#include "cJSON.h"

int parse_record_ip(const char *response, char *ip, int ip_length) {
  cJSON *root = cJSON_Parse(response);

  // Response is not valid JSON
  if (root == NULL) {
    return DDNS_ERR_PARSE;
  }

  // The API responded, but with success:false (wrong token/permission/record)
  cJSON *success = cJSON_GetObjectItemCaseSensitive(root, "success");
  if (!cJSON_IsTrue(success)) {
    cJSON_Delete(root);

    return DDNS_ERR_API;
  }

  // Go to: root -> result -> content
  cJSON *result = cJSON_GetObjectItemCaseSensitive(root, "result");
  cJSON *content = cJSON_GetObjectItemCaseSensitive(result, "content");

  // Invalid or missing content field in the response
  if (!cJSON_IsString(content) || content->valuestring == NULL) {
    cJSON_Delete(root);

    return DDNS_ERR_PARSE;
  }

  snprintf(ip, ip_length, "%s", content->valuestring);

  cJSON_Delete(root);
  
  return DDNS_OK;
}