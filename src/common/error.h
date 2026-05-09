/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#ifndef ERROR_H
#define ERROR_H

/**
 * System-wide return codes
 *
 * Design:
 * - 0 = Success (RC_OK)
 * - Negative values = Errors
 * - Errors grouped by module in ranges of 100
 * - First digit indicates module, last two digits are specific errors
 *
 * Ranges:
 * -1xx  : Common/Generic errors
 * -2xx  : WiFi module errors
 * -3xx  : HTTP module errors
 * -4xx  : Flash/Storage errors
 * -5xx  : Server errors
 */

typedef enum
{
    // =========================================================================
    // SUCCESS
    // =========================================================================
    RC_OK = 0,

    // =========================================================================
    // COMMON ERRORS (-1xx)
    // =========================================================================
    RC_ERR_INVALID_PARAM = -100,    // Invalid function parameter
    RC_ERR_NULL_POINTER = -101,     // Null pointer provided
    RC_ERR_OUT_OF_MEMORY = -102,    // Memory allocation failed
    RC_ERR_TIMEOUT = -103,          // Operation timed out
    RC_ERR_BUSY = -104,             // Resource is busy
    RC_ERR_NOT_INITIALIZED = -105,  // Module not initialized
    RC_ERR_ALREADY_INIT = -106,     // Already initialized
    RC_ERR_UNKNOWN = -199,          // Unknown/unspecified error

    // =========================================================================
    // WIFI MODULE ERRORS (-2xx)
    // =========================================================================
    RC_WIFI_INIT_FAILED = -200,        // WiFi hardware init failed
    RC_WIFI_CONNECT_FAILED = -201,     // Generic connection failure
    RC_WIFI_BAD_AUTH = -202,           // Authentication failed (wrong password)
    RC_WIFI_NO_NETWORK = -203,         // Network not found (wrong SSID)
    RC_WIFI_TIMEOUT = -204,            // Connection timeout
    RC_WIFI_ALREADY_CONNECTED = -205,  // Already connected (not an error, informational)
    RC_WIFI_DISCONNECTED = -206,       // Unexpected disconnection
    RC_WIFI_AP_FAILED = -207,          // Access point mode failed

    // =========================================================================
    // HTTP MODULE ERRORS (-3xx)
    // =========================================================================
    RC_HTTP_INVALID_PARAMS = -300,  // Invalid parameters (NULL host/path)
    RC_HTTP_ALREADY_ACTIVE = -301,  // Request already in progress
    RC_HTTP_ALLOC_FAILED = -302,    // Memory allocation failed
    RC_HTTP_DNS_FAILED = -303,      // DNS lookup failed
    RC_HTTP_CONNECT_FAILED = -304,  // TCP connection failed
    RC_HTTP_WRITE_FAILED = -305,    // TCP write failed
    RC_HTTP_OUTPUT_FAILED = -306,   // TCP output failed
    RC_HTTP_TIMEOUT = -307,         // HTTP request timeout
    RC_HTTP_ERROR = -308,           // General HTTP protocol error
    RC_HTTP_BUFFER_FULL = -309,     // Response buffer full

    // =========================================================================
    // FLASH/STORAGE ERRORS (-4xx)
    // =========================================================================
    RC_FLASH_MAGIC_INVALID = -400,  // Magic number mismatch (no valid config)
    RC_FLASH_CRC_FAILED = -401,     // CRC check failed (corrupted data)
    RC_FLASH_WRITE_FAILED = -402,   // Flash write operation failed
    RC_FLASH_ERASE_FAILED = -403,   // Flash erase operation failed
    RC_FLASH_NOT_FOUND = -404,      // No flash config exists

    // =========================================================================
    // SERVER ERRORS (-5xx)
    // =========================================================================
    RC_SERVER_BIND_FAILED = -500,    // Failed to bind to port
    RC_SERVER_LISTEN_FAILED = -501,  // Failed to listen
    RC_SERVER_ACCEPT_FAILED = -502,  // Failed to accept connection
    RC_SERVER_SEND_FAILED = -503,    // Failed to send response

    // =========================================================================
    // PARSER ERRORS (-6xx)
    // =========================================================================
    RC_PARSER_TOO_FEW_COLUMNS = -600,    // Row has fewer columns than the selected format requires
    RC_PARSER_NO_POSITIONS = -602,        // File parsed but no valid positions were found
    RC_PARSER_UNRECOGNIZED_FORMAT = -603,  // Doesn't match any known broker format
    RC_PARSER_TOO_MANY_TICKERS = -604,    // Portfolio exceeds the 50-position limit

} ReturnCode;

#define RC_SUCCESS(rc) ((rc) == RC_OK)
#define RC_FAILED(rc) ((rc) != RC_OK)
#define RC_IS_WIFI_ERROR(rc) ((rc) <= -200 && (rc) > -300)
#define RC_IS_HTTP_ERROR(rc) ((rc) <= -300 && (rc) > -400)
#define RC_IS_FLASH_ERROR(rc) ((rc) <= -400 && (rc) > -500)
#define RC_IS_SERVER_ERROR(rc) ((rc) <= -500 && (rc) > -600)
#define RC_IS_PARSER_ERROR(rc) ((rc) <= -600 && (rc) > -700)

const char* rc_to_string(ReturnCode rc);

#endif  // ERROR_H
