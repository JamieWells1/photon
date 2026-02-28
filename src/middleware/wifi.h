#ifndef WIFI_H
#define WIFI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum
{
    WIFI_DISCONNECTED,
    WIFI_CONNECTING,
    WIFI_CONNECTED,
    WIFI_FAILED
} WifiState;

int wifi_join(const char* ssid, const char* password);
int wifi_join_async(const char* ssid, const char* password);
WifiState wifi_get_state(void);
void wifi_disconnect(void);

// ==========================
// WiFi Connection Manager
// ==========================

typedef struct
{
    int attempt_count;
    int max_attempts;
    bool failed;
    bool in_progress;
} WifiConnectionManager;

void wifi_manager_init(WifiConnectionManager* mgr, int max_attempts);
void wifi_manager_start(WifiConnectionManager* mgr, const char* ssid, const char* password);

WifiState wifi_manager_update(WifiConnectionManager* mgr, const char* ssid, const char* password);

bool wifi_manager_is_connecting(const WifiConnectionManager* mgr);
bool wifi_manager_has_failed(const WifiConnectionManager* mgr);

int wifi_manager_get_attempts(const WifiConnectionManager* mgr);

void wifi_manager_reset(WifiConnectionManager* mgr);

// High-level helper to check if we should start a connection
bool wifi_manager_should_connect(const WifiConnectionManager* mgr);

// High-level helper to start connection if needed
void wifi_manager_connect_if_needed(WifiConnectionManager* mgr, const char* ssid,
                                     const char* password, int max_attempts);

// Auto-disconnect if connected (convenience wrapper)
void wifi_manager_disconnect_if_connected(void);

void wifi_append_connecting_dots(char* buffer, size_t buffer_size, int* dot_counter);

#endif  // WIFI_H
