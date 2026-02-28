#ifndef WIFI_H
#define WIFI_H

#include <matrix.h>

int wifi_connect(const char* ssid, const char* password, Matrix* mtrx);
void wifi_disconnect();

#endif  // WIFI_H
