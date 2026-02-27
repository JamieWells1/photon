#ifndef SECRETS_CONFIG_H
#define SECRETS_CONFIG_H

// Include secrets.h if it exists, otherwise use placeholders
#if __has_include(<secrets.h>)
#include <secrets.h>
#else
#warning "secrets.h not found. Copy secrets.example.h to secrets.h and add credentials."
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#endif

#endif  // SECRETS_CONFIG_H
