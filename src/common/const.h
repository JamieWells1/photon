#ifndef CONST_H
#define CONST_H

// Log level
typedef enum
{
    TRACE = 2,
    INPUTS = 1,
    INFO = 0,
} DebugLevel;

extern const DebugLevel DEBUG_LEVEL;

// Display
#define PROJECT_NAME "PHOTON"
#define DISPLAY_UPDATE_INTERVAL_TICKS_MS 50

// Matrix layout
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 8
#define NUM_PIXELS (MATRIX_WIDTH * MATRIX_HEIGHT)

// Matrix pixels
#define BRIGHTNESS_COEFFICIENT 0.02

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

// Weather
#define WEATHER_HOURS 48

// Locations + timezones
#define LOCATION LOC_TOKYO

#define LAT_LONDON 51.6611
#define LAT_MIAMI 25.7617
#define LAT_LOS_ANGELES 34.0522
#define LAT_CHICAGO 41.8781
#define LAT_TOKYO 35.6762
#define LAT_SYDNEY -33.8688
#define LAT_STOCKHOLM 59.3293
#define LAT_RIO_DE_JANEIRO -22.9068
#define LAT_CAPE_TOWN -33.9249
#define LAT_ATHENS 37.9838
#define LAT_DEFAULT 0

#define LON_LONDON 0.3970
#define LON_MIAMI -80.1918
#define LON_LOS_ANGELES -118.2437
#define LON_CHICAGO -87.6298
#define LON_TOKYO 139.6503
#define LON_SYDNEY 151.2093
#define LON_STOCKHOLM 18.0686
#define LON_RIO_DE_JANEIRO -43.1729
#define LON_CAPE_TOWN 18.4241
#define LON_ATHENS 23.7275
#define LON_DEFAULT 0

typedef enum
{
    LOC_LONDON,
    LOC_MIAMI,
    LOC_LOS_ANGELES,
    LOC_CHICAGO,
    LOC_TOKYO,
    LOC_SYDNEY,
    LOC_STOCKHOLM,
    LOC_RIO_DE_JANEIRO,
    LOC_CAPE_TOWN,
    LOC_ATHENS,
} Location;

#endif  // CONST_H
