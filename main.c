#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"
#include "ws_client.h"


void usage(char *app) {
  printf("Usage: %s config_file audio_file\n\
Ex: %s /root/tmp/olaris.json konnichiwa-pad.raw\n\
", app, app);
}

int main(int argc, char *argv[]) {
  if(argc != 3) {
    usage(argv[0]);
    exit(1);
  }

  char *config_file = argv[1];
  char *audio_file = argv[2];

  char api_key[1024];
  char product_name[1024];
  char organization_id[1024];

  get_config(config_file, api_key, product_name, organization_id);

  //printf("config data api_key=%s product_name=%s organization_id=%s\n", api_key, product_name, organization_id);

  char access_token[2048];
  get_access_token(api_key, product_name, organization_id, access_token);

  //printf("access_token: %s\n", access_token);

  ws_client_start(access_token, product_name, organization_id, audio_file);
  while (1) {
    //printf(".");
    //fflush(stdout);
    ws_client_loop();
    sleep(0.1);
  }
  return 0;
}
