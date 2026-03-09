/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#ifndef WIFI_H
#define WIFI_H

#include <error.h>
#include <matrix.h>

#include <stdbool.h>

ReturnCode wifi_connect(const char* ssid, const char* password, Matrix* mtrx);
void wifi_disconnect(void);
ReturnCode wifi_init_ap(const char* ssid, const char* password, Matrix* mtrx);
bool wifi_is_connected(void);
void wifi_start_mdns(void);

#endif  // WIFI_H
