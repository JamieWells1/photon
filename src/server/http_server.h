#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <stdbool.h>

void http_server_start(void);
void http_server_stop(void);
bool http_server_credentials_saved(void);

#endif  // HTTP_SERVER_H
