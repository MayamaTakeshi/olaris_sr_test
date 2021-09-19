#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <curl/curl.h>
#include <libwebsockets.h>

#include "util.h"

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(ptr == NULL) {
    printf("error: not enough memory\n");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

void get_config(char *config_file, char *api_key, char *product_name, char *organization_id) {
  FILE *fd;

  if((fd = fopen(config_file, "r")) < 0) {
    fprintf(stderr, "open(): '%s'\n", strerror(errno));
    exit(1);
  }

  struct stat info;
  stat(config_file, &info);

  char buff[2048];

  fread(buff, 1, info.st_size, fd);
  fclose(fd);

  struct json_object *jobj;
  jobj = json_tokener_parse(buff);
  //printf("jobj from str:\n---\n%s\n---\n", json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));

  struct json_object *tmp;

  json_object_object_get_ex(jobj, "api_key", &tmp);
  if(!tmp) {
    fprintf(stderr, "failed to get %s\n", "api_key");
    exit(1);
  }
  strcpy(api_key, json_object_get_string(tmp));

  json_object_object_get_ex(jobj, "product_name", &tmp);
  if(!tmp) {
    fprintf(stderr, "failed to get %s\n", "product_nam");
    exit(1);
  }
  strcpy(product_name, json_object_get_string(tmp));

  json_object_object_get_ex(jobj, "organization_id", &tmp);
  if(!tmp) {
    fprintf(stderr, "failed to get %s\n", "organization_id");
    exit(1);
  }
  strcpy(organization_id, json_object_get_string(tmp));
}

void get_access_token(char *api_key, char *product_name, char *organization_id, char *access_token) {
  char url[] = "https://realtime.stt.batoner.works/real-time-decode/v1/issue_token/";

  CURL *curl_handle;
  CURLcode res;

  struct MemoryStruct chunk;
  chunk.memory = malloc(1);  
  chunk.size = 0;

  curl_handle = curl_easy_init();
  if(curl_handle) {
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    struct curl_slist *list = NULL;
    list = curl_slist_append(list, "accept: text/html");

    char buff[4096];
    sprintf(buff, "Authorization: Bearer %s", api_key);
    list = curl_slist_append(list, buff);
    list = curl_slist_append(list, "Content-Type: application/json");
 
    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, list);

    sprintf(buff, "{\"product_name\": \"%s\", \"organization_id\": \"%s\"}", product_name, organization_id);
    //printf("POST DATA: %s\n", buff);

    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, buff);

    res = curl_easy_perform(curl_handle);

    if(res != CURLE_OK) {
      fprintf(stderr, "error: %s\n", curl_easy_strerror(res));
      exit(1);
    } else {
      long response_code;
      curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);
      printf("Response Code: %li\n", response_code);
      if(response_code != 200) {
        fprintf(stderr, "Unexpected Response Code: %li\n", response_code);
        exit(1);
      }

      printf("Size: %lu\n", (unsigned long)chunk.size);
      //printf("Data: %s\n", chunk.memory);

      strcpy(access_token, chunk.memory);
    }
    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
  }
}
