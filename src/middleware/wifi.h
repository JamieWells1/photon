#ifndef WIFI_H
#define WIFI_H

#include <matrix.h>
#include <stdbool.h>

int wifi_connect(const char* ssid, const char* password, Matrix* mtrx);
void wifi_disconnect(void);
int wifi_init_ap(const char* ssid, const char* password, Matrix* mtrx);
bool wifi_is_ap_mode(void);

#endif  // WIFI_H
