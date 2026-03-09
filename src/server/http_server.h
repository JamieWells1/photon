/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <stdbool.h>

void http_server_start(void);
void http_server_stop(void);
bool http_server_credentials_saved(void);
bool http_server_config_saved(void);
bool http_server_portfolio_parse_failed(void);

#endif  // HTTP_SERVER_H
