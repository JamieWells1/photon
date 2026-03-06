#include <error.h>

const char* rc_to_string(ReturnCode rc)
{
    switch (rc)
    {
        // Success
        case RC_OK:
            return "OK";

        // Common errors
        case RC_ERR_INVALID_PARAM:
            return "Invalid parameter";
        case RC_ERR_NULL_POINTER:
            return "Null pointer";
        case RC_ERR_OUT_OF_MEMORY:
            return "Out of memory";
        case RC_ERR_TIMEOUT:
            return "Timeout";
        case RC_ERR_BUSY:
            return "Resource busy";
        case RC_ERR_NOT_INITIALIZED:
            return "Not initialized";
        case RC_ERR_ALREADY_INIT:
            return "Already initialized";
        case RC_ERR_UNKNOWN:
            return "Unknown error";

        // WiFi errors
        case RC_WIFI_INIT_FAILED:
            return "WiFi: Init failed";
        case RC_WIFI_CONNECT_FAILED:
            return "WiFi: Connection failed";
        case RC_WIFI_BAD_AUTH:
            return "WiFi: Bad credentials";
        case RC_WIFI_NO_NETWORK:
            return "WiFi: Network not found";
        case RC_WIFI_TIMEOUT:
            return "WiFi: Timeout";
        case RC_WIFI_ALREADY_CONNECTED:
            return "WiFi: Already connected";
        case RC_WIFI_DISCONNECTED:
            return "WiFi: Disconnected";
        case RC_WIFI_AP_FAILED:
            return "WiFi: AP mode failed";

        // HTTP errors
        case RC_HTTP_INVALID_PARAMS:
            return "HTTP: Invalid params";
        case RC_HTTP_ALREADY_ACTIVE:
            return "HTTP: Already active";
        case RC_HTTP_ALLOC_FAILED:
            return "HTTP: Alloc failed";
        case RC_HTTP_DNS_FAILED:
            return "HTTP: DNS failed";
        case RC_HTTP_CONNECT_FAILED:
            return "HTTP: Connect failed";
        case RC_HTTP_WRITE_FAILED:
            return "HTTP: Write failed";
        case RC_HTTP_OUTPUT_FAILED:
            return "HTTP: Output failed";
        case RC_HTTP_TIMEOUT:
            return "HTTP: Timeout";
        case RC_HTTP_ERROR:
            return "HTTP: Protocol error";
        case RC_HTTP_BUFFER_FULL:
            return "HTTP: Buffer full";

        // Flash errors
        case RC_FLASH_MAGIC_INVALID:
            return "Flash: Invalid magic";
        case RC_FLASH_CRC_FAILED:
            return "Flash: CRC failed";
        case RC_FLASH_WRITE_FAILED:
            return "Flash: Write failed";
        case RC_FLASH_ERASE_FAILED:
            return "Flash: Erase failed";
        case RC_FLASH_NOT_FOUND:
            return "Flash: Not found";

        // Server errors
        case RC_SERVER_BIND_FAILED:
            return "Server: Bind failed";
        case RC_SERVER_LISTEN_FAILED:
            return "Server: Listen failed";
        case RC_SERVER_ACCEPT_FAILED:
            return "Server: Accept failed";
        case RC_SERVER_SEND_FAILED:
            return "Server: Send failed";

        default:
            return "Unknown return code";
    }
}
