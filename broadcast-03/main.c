#include <stdio.h>
#include <cjson/cJSON.h>
#include <string.h>
#include <stdlib.h>
#include "uthash.h"

void handle_message(const cJSON *root);
void send_init(const cJSON *src, const cJSON *dest, const cJSON *msg_id);
void send_echo(const cJSON *src, const cJSON *dest, const cJSON *msg_id, const cJSON *echo);
void send_generate(const cJSON *src, const cJSON *dest, const cJSON *msg_id, const int node_id);
void send_broadcast(const cJSON *src, const cJSON *dest, const cJSON *msg_id, const int message);

int unique_idx = 0;
int total_node_count = 0;

int main(void) {
  char buffer[4096];
  
  while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
    fprintf(stderr, "received: %s\n", buffer);

    cJSON *root = cJSON_Parse(buffer);

    if (root == NULL) {
      continue;
    }

    handle_message(root);

    cJSON_Delete(root);
  }


  return 0;
}

void handle_message(const cJSON *root) {
    cJSON *src = cJSON_GetObjectItem(root, "src");
    cJSON *dest = cJSON_GetObjectItem(root, "dest");
    cJSON *body = cJSON_GetObjectItem(root, "body");
    cJSON *type = cJSON_GetObjectItem(body, "type");
    cJSON *msg_id = cJSON_GetObjectItem(body, "msg_id");
    cJSON *echo = cJSON_GetObjectItem(body, "echo");


    if (cJSON_IsString(type) && strcmp(type->valuestring, "init") == 0) {
      cJSON *nodes = cJSON_GetObjectItemCaseSensitive(body, "node_ids");
      total_node_count = cJSON_GetArraySize(nodes);
      send_init(src, dest, msg_id);
    } else if (cJSON_IsString(type) && strcmp(type->valuestring, "echo") == 0) {
      send_echo(src, dest, msg_id, echo);
    } else if (cJSON_IsString(type) && strcmp(type->valuestring, "generate") == 0) {
      char* node = dest->valuestring;
      int node_id = (int) strtol(node + 1, NULL, 10);
      fprintf(stderr, "parsed int %s", msg_id);
      send_generate(src, dest, msg_id, node_id);
    } else if (cJSON_IsString(type) && strcmp(type->valuestring, "broadcast") == 0) {
      cJSON *message_json = cJSON_GetObjectItem(body, "message");
      int message = message_json->valueint;
      send_generate(src, dest, msg_id, message);
    } else {
      fprintf(stderr, "else case");
    }

}

void send_init(const cJSON *src, const cJSON *dest, const cJSON *msg_id) {
  cJSON *response = cJSON_CreateObject();
  cJSON_AddStringToObject(response, "src", dest->valuestring);
  cJSON_AddStringToObject(response, "dest", src->valuestring);

  cJSON *body = cJSON_CreateObject();
  cJSON_AddStringToObject(body, "type", "init_ok");
  // cJSON_AddStringToObject(body, "in_reply_to", 1);
  cJSON_AddNumberToObject(body, "in_reply_to", msg_id->valueint);
  cJSON_AddItemToObject(response, "body", body);
  
  char *json = cJSON_PrintUnformatted(response);

  if (json != NULL) {
    printf("%s\n", json);
    fprintf(stderr, "SEND INIT %s", json);
    fflush(stdout);
    cJSON_free(json);
  }

  cJSON_Delete(response);
}

void send_echo(const cJSON *src, const cJSON *dest, const cJSON *msg_id, const cJSON *echo) {
  cJSON *response = cJSON_CreateObject();
  cJSON_AddStringToObject(response, "src", dest->valuestring);
  cJSON_AddStringToObject(response, "dest", src->valuestring);

  cJSON *body = cJSON_CreateObject();
  cJSON_AddStringToObject(body, "type", "echo_ok");
  cJSON_AddNumberToObject(body, "in_reply_to", msg_id->valueint);
  cJSON_AddItemToObject(body, "echo", cJSON_Duplicate(echo, 1));
  cJSON_AddItemToObject(response, "body", body);
  
  char *json = cJSON_PrintUnformatted(response);

  if (json != NULL) {
    printf("%s\n", json);
    fflush(stdout);
    cJSON_free(json);
  }

  cJSON_Delete(response);
}

void send_generate(const cJSON *src, const cJSON *dest, const cJSON *msg_id, const int node_id) {
  cJSON *response = cJSON_CreateObject();
  cJSON_AddStringToObject(response, "src", dest->valuestring);
  cJSON_AddStringToObject(response, "dest", src->valuestring);

  cJSON *body = cJSON_CreateObject();
  cJSON_AddStringToObject(body, "type", "generate_ok");
  cJSON_AddNumberToObject(body, "in_reply_to", msg_id->valueint);

  int unique_id = unique_idx * total_node_count + node_id;
  unique_idx += 1;

  cJSON_AddNumberToObject(body, "id", unique_id);
  cJSON_AddItemToObject(response, "body", body);

  char *json = cJSON_PrintUnformatted(response);

  if (json != NULL) {
    printf("%s\n", json);
    fflush(stdout);
    cJSON_free(json);
  }

  cJSON_Delete(response);
}
