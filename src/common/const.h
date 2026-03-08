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

extern const LogLevel LOG_LEVEL;

// Device defaults
#define DEFAULT_BRIGHTNESS 8   // 8%
#define DEFAULT_LOCATION 0     // London
#define DEFAULT_CURRENCY 0     // GBP
#define DEFAULT_CRYPTO 0x0007  // XBT + ETH + SOL
#define DEFAULT_STOCKS 0x0007  // AAPL + MSFT + GOOGL

// Brightness limits by power source
#define MAX_BRIGHTNESS_USB 5
#define MAX_BRIGHTNESS_MAINS 20

// Display
#define PROJECT_NAME "PHOTON"
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
#define MAX_WIFI_CONNECTION_ATTEMPTS 100

// HTTP
#define PROXY_HOST "138.68.190.163"
#define PROXY_PORT "8080"

#define HOST_URL_WEATHER "api.open-meteo.com"
#define HOST_URL_CRYPTO "api.kraken.com"

// Weather
#define WEATHER_HOURS 48

// Location data
#define NUM_LOCATIONS 18
extern const char* const LOCATION_NAMES[NUM_LOCATIONS];

// Currency data
#define NUM_CURRENCIES 12
extern const char* const CURRENCY_NAMES[NUM_CURRENCIES];
extern const char* const CURRENCY_SYMBOLS[NUM_CURRENCIES];

// Stock data
#define NUM_STOCK_TICKERS 10
extern const char* const STOCK_TICKER_NAMES[NUM_STOCK_TICKERS];

// Crypto data
#define NUM_CRYPTO_TICKERS 10
extern const char* const CRYPTO_TICKER_NAMES[NUM_CRYPTO_TICKERS];

#endif  // CONST_H
