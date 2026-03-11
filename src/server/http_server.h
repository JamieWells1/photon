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
bool http_server_update_triggered(void);

// Ticker validation state: 0=idle, 1=pending, 2=valid, 3=invalid
bool http_server_has_pending_validation(void);
const char* http_server_get_pending_ticker(void);
void http_server_set_validation_result(bool valid);

#endif  // HTTP_SERVER_H
