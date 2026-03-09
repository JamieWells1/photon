/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#ifndef LOCATION_H
#define LOCATION_H

#define LAT_LONDON 51.6611
#define LAT_NEW_YORK 40.7128
#define LAT_LOS_ANGELES 34.0522
#define LAT_CHICAGO 41.8781
#define LAT_MIAMI 25.7617
#define LAT_SAN_FRANCISCO 37.7749
#define LAT_SEATTLE 47.6062
#define LAT_TORONTO 43.6532
#define LAT_PARIS 48.8566
#define LAT_BERLIN 52.5200
#define LAT_STOCKHOLM 59.3293
#define LAT_ATHENS 37.9838
#define LAT_DUBAI 25.2048
#define LAT_SINGAPORE 1.3521
#define LAT_TOKYO 35.6762
#define LAT_SYDNEY -33.8688
#define LAT_RIO_DE_JANEIRO -22.9068
#define LAT_CAPE_TOWN -33.9249
#define LAT_DEFAULT 0

#define LON_LONDON 0.3970
#define LON_NEW_YORK -74.0060
#define LON_LOS_ANGELES -118.2437
#define LON_CHICAGO -87.6298
#define LON_MIAMI -80.1918
#define LON_SAN_FRANCISCO -122.4194
#define LON_SEATTLE -122.3321
#define LON_TORONTO -79.3832
#define LON_PARIS 2.3522
#define LON_BERLIN 13.4050
#define LON_STOCKHOLM 18.0686
#define LON_ATHENS 23.7275
#define LON_DUBAI 55.2708
#define LON_SINGAPORE 103.8198
#define LON_TOKYO 139.6503
#define LON_SYDNEY 151.2093
#define LON_RIO_DE_JANEIRO -43.1729
#define LON_CAPE_TOWN 18.4241
#define LON_DEFAULT 0

typedef enum
{
    LOC_LONDON,
    LOC_NEW_YORK,
    LOC_LOS_ANGELES,
    LOC_CHICAGO,
    LOC_MIAMI,
    LOC_SAN_FRANCISCO,
    LOC_SEATTLE,
    LOC_TORONTO,
    LOC_PARIS,
    LOC_BERLIN,
    LOC_STOCKHOLM,
    LOC_ATHENS,
    LOC_DUBAI,
    LOC_SINGAPORE,
    LOC_TOKYO,
    LOC_SYDNEY,
    LOC_RIO_DE_JANEIRO,
    LOC_CAPE_TOWN,
    LOC_COUNT
} Location;

float location_get_latitude(Location loc);
float location_get_longitude(Location loc);
const char* location_get_timezone(Location loc);
const char* location_get_name(Location loc);

#endif  // LOCATION_H
