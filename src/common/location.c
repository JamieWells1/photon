#include <location.h>

float location_get_latitude(Location loc)
{
    switch (loc)
    {
        case LOC_LONDON:
            return LAT_LONDON;
        case LOC_NEW_YORK:
            return LAT_NEW_YORK;
        case LOC_LOS_ANGELES:
            return LAT_LOS_ANGELES;
        case LOC_CHICAGO:
            return LAT_CHICAGO;
        case LOC_MIAMI:
            return LAT_MIAMI;
        case LOC_SAN_FRANCISCO:
            return LAT_SAN_FRANCISCO;
        case LOC_SEATTLE:
            return LAT_SEATTLE;
        case LOC_TORONTO:
            return LAT_TORONTO;
        case LOC_PARIS:
            return LAT_PARIS;
        case LOC_BERLIN:
            return LAT_BERLIN;
        case LOC_STOCKHOLM:
            return LAT_STOCKHOLM;
        case LOC_ATHENS:
            return LAT_ATHENS;
        case LOC_DUBAI:
            return LAT_DUBAI;
        case LOC_SINGAPORE:
            return LAT_SINGAPORE;
        case LOC_TOKYO:
            return LAT_TOKYO;
        case LOC_SYDNEY:
            return LAT_SYDNEY;
        case LOC_RIO_DE_JANEIRO:
            return LAT_RIO_DE_JANEIRO;
        case LOC_CAPE_TOWN:
            return LAT_CAPE_TOWN;
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
        case LOC_NEW_YORK:
            return LON_NEW_YORK;
        case LOC_LOS_ANGELES:
            return LON_LOS_ANGELES;
        case LOC_CHICAGO:
            return LON_CHICAGO;
        case LOC_MIAMI:
            return LON_MIAMI;
        case LOC_SAN_FRANCISCO:
            return LON_SAN_FRANCISCO;
        case LOC_SEATTLE:
            return LON_SEATTLE;
        case LOC_TORONTO:
            return LON_TORONTO;
        case LOC_PARIS:
            return LON_PARIS;
        case LOC_BERLIN:
            return LON_BERLIN;
        case LOC_STOCKHOLM:
            return LON_STOCKHOLM;
        case LOC_ATHENS:
            return LON_ATHENS;
        case LOC_DUBAI:
            return LON_DUBAI;
        case LOC_SINGAPORE:
            return LON_SINGAPORE;
        case LOC_TOKYO:
            return LON_TOKYO;
        case LOC_SYDNEY:
            return LON_SYDNEY;
        case LOC_RIO_DE_JANEIRO:
            return LON_RIO_DE_JANEIRO;
        case LOC_CAPE_TOWN:
            return LON_CAPE_TOWN;
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
        case LOC_NEW_YORK:
            return "America/New_York";
        case LOC_LOS_ANGELES:
            return "America/Los_Angeles";
        case LOC_CHICAGO:
            return "America/Chicago";
        case LOC_MIAMI:
            return "America/New_York";
        case LOC_SAN_FRANCISCO:
            return "America/Los_Angeles";
        case LOC_SEATTLE:
            return "America/Los_Angeles";
        case LOC_TORONTO:
            return "America/Toronto";
        case LOC_PARIS:
            return "Europe/Paris";
        case LOC_BERLIN:
            return "Europe/Berlin";
        case LOC_STOCKHOLM:
            return "Europe/Stockholm";
        case LOC_ATHENS:
            return "Europe/Athens";
        case LOC_DUBAI:
            return "Asia/Dubai";
        case LOC_SINGAPORE:
            return "Asia/Singapore";
        case LOC_TOKYO:
            return "Asia/Tokyo";
        case LOC_SYDNEY:
            return "Australia/Sydney";
        case LOC_RIO_DE_JANEIRO:
            return "America/Sao_Paulo";
        case LOC_CAPE_TOWN:
            return "Africa/Johannesburg";
        default:
            return "UTC";
    }
}

const char* location_get_name(Location loc)
{
    switch (loc)
    {
        case LOC_LONDON:
            return "London";
        case LOC_NEW_YORK:
            return "New York";
        case LOC_LOS_ANGELES:
            return "Los Angeles";
        case LOC_CHICAGO:
            return "Chicago";
        case LOC_MIAMI:
            return "Miami";
        case LOC_SAN_FRANCISCO:
            return "San Francisco";
        case LOC_SEATTLE:
            return "Seattle";
        case LOC_TORONTO:
            return "Toronto";
        case LOC_PARIS:
            return "Paris";
        case LOC_BERLIN:
            return "Berlin";
        case LOC_STOCKHOLM:
            return "Stockholm";
        case LOC_ATHENS:
            return "Athens";
        case LOC_DUBAI:
            return "Dubai";
        case LOC_SINGAPORE:
            return "Singapore";
        case LOC_TOKYO:
            return "Tokyo";
        case LOC_SYDNEY:
            return "Sydney";
        case LOC_RIO_DE_JANEIRO:
            return "Rio de Janeiro";
        case LOC_CAPE_TOWN:
            return "Cape Town";
        default:
            return "Unknown";
    }
}
