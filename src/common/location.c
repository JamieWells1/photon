#include <location.h>

float location_get_latitude(Location loc)
{
    switch (loc)
    {
        case LOC_LONDON:
            return LAT_LONDON;
        case LOC_MIAMI:
            return LAT_MIAMI;
        case LOC_LOS_ANGELES:
            return LAT_LOS_ANGELES;
        case LOC_CHICAGO:
            return LAT_CHICAGO;
        case LOC_TOKYO:
            return LAT_TOKYO;
        case LOC_SYDNEY:
            return LAT_SYDNEY;
        case LOC_STOCKHOLM:
            return LAT_STOCKHOLM;
        case LOC_RIO_DE_JANEIRO:
            return LAT_RIO_DE_JANEIRO;
        case LOC_CAPE_TOWN:
            return LAT_CAPE_TOWN;
        case LOC_ATHENS:
            return LAT_ATHENS;
        default:
            return 0.0;
    }
}

float location_get_longitude(Location loc)
{
    switch (loc)
    {
        case LOC_LONDON:
            return LON_LONDON;
        case LOC_MIAMI:
            return LON_MIAMI;
        case LOC_LOS_ANGELES:
            return LON_LOS_ANGELES;
        case LOC_CHICAGO:
            return LON_CHICAGO;
        case LOC_TOKYO:
            return LON_TOKYO;
        case LOC_SYDNEY:
            return LON_SYDNEY;
        case LOC_STOCKHOLM:
            return LON_STOCKHOLM;
        case LOC_RIO_DE_JANEIRO:
            return LON_RIO_DE_JANEIRO;
        case LOC_CAPE_TOWN:
            return LON_CAPE_TOWN;
        case LOC_ATHENS:
            return LON_ATHENS;
        default:
            return 0.0;
    }
}

const char* location_get_timezone(Location loc)
{
    switch (loc)
    {
        case LOC_LONDON:
            return "Europe/London";
        case LOC_MIAMI:
            return "America/New_York";
        case LOC_LOS_ANGELES:
            return "America/Los_Angeles";
        case LOC_CHICAGO:
            return "America/Chicago";
        case LOC_TOKYO:
            return "Asia/Tokyo";
        case LOC_SYDNEY:
            return "Australia/Sydney";
        case LOC_STOCKHOLM:
            return "Europe/Stockholm";
        case LOC_RIO_DE_JANEIRO:
            return "America/Sao_Paulo";
        case LOC_CAPE_TOWN:
            return "Africa/Johannesburg";
        case LOC_ATHENS:
            return "Europe/Athens";
        default:
            return "UTC";
    }
}
