#ifndef WS_CLIENT_H
#define WS_CLIENT_H

void ws_client_start(char *access_token, char *product_name, char *organization_id, char *audio_file);
void ws_client_loop();

#endif
