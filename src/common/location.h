#ifndef LOCATION_H
#define LOCATION_H

// Define location you want to use
#define LOCATION LOC_LONDON

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

float location_get_latitude(Location loc);
float location_get_longitude(Location loc);
const char* location_get_timezone(Location loc);

#endif  // LOCATION_H
