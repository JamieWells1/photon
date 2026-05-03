/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#ifndef CONST_H
#define CONST_H

// Log level
typedef enum
{
    ERROR = 0,
    WARN = 1,
    INFO = 2,
    DEBUG = 3,
    TRACE = 4
} LogLevel;

#define LOG_LEVEL TRACE

// Build version (injected by CMake)
#ifndef BUILD_VERSION
#define BUILD_VERSION "20260101"  // Fallback for IDE
#endif

// Device defaults
#define DEFAULT_BRIGHTNESS 20  // 20%
#define DEFAULT_LOCATION 0     // London
#define DEFAULT_CURRENCY 0     // GBP
#define DEFAULT_CRYPTO 0x0007  // XBT + ETH + SOL
#define DEFAULT_STOCKS 0x0000  // No default stocks (dynamic tickers)

// Brightness limits
#define MIN_BRIGHTNESS 1
#define MAX_BRIGHTNESS 25

// Display
#define PROJECT_NAME "DELTA"
#define DISPLAY_UPDATE_INTERVAL_TICKS_MS 30

// Matrix layout
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 8
#define NUM_PIXELS (MATRIX_WIDTH * MATRIX_HEIGHT)

// GPIO pins
#define BUTTON_LEFT_PIN 10  // GP10
#define BUTTON_RIGHT_PIN 5  // GP5

#define ROTATOR_CLK_PIN 2  // GP2
#define ROTATOR_DT_PIN 3   // GP3
#define ROTATOR_SW_PIN 4   // GP4

#define MATRIX_DIN_PIN 21  // GP21
#define MATRIX_DATA_FREQ 800000

// Wifi
#define MAX_WIFI_CONNECTION_ATTEMPTS 200

// HTTP
#define PROXY_HOST "138.68.190.163"
#define PROXY_PORT "8080"

#define HOST_URL_WEATHER "api.open-meteo.com"
#define HOST_URL_CRYPTO "api.kraken.com"

// Weather
#define WEATHER_HOURS 48
#define WEATHER_FETCH_INTERVAL_MINUTES 60

// All tickers
#define TKR_FETCH_INTERVAL_MINUTES 10

#endif  // CONST_H
