#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <json-c/json.h>

struct MemoryStruct {
  char *memory;
  size_t size;
};

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

void get_config(char *config_file, char *api_key, char *product_name, char *organization_id);

void get_access_token(char *api_key, char *product_name, char *organization_id, char *access_token);

#endif
