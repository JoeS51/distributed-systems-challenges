#include <stdio.h>
#include <cjson/cJSON.h>
#include <string.h>

void handle_message(const cJSON *root);
void send_init(const cJSON *src, const cJSON *dest, const cJSON *msg_id);
void send_echo(const cJSON *src, const cJSON *dest, const cJSON *msg_id, const cJSON *echo);

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
      send_init(src, dest, msg_id);
    } else if (cJSON_IsString(type) && strcmp(type->valuestring, "echo") == 0) {
      send_echo(src, dest, msg_id, echo);
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
