/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#include <location.h>

float location_get_latitude(Location loc)
{
    switch (loc)
    {
        case LOC_LONDON:
            return LAT_LONDON;
        case LOC_MANCHESTER:
            return LAT_MANCHESTER;
        case LOC_EDINBURGH:
            return LAT_EDINBURGH;
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
        case LOC_DUBLIN:
            return LAT_DUBLIN;
        case LOC_PARIS:
            return LAT_PARIS;
        case LOC_AMSTERDAM:
            return LAT_AMSTERDAM;
        case LOC_BRUSSELS:
            return LAT_BRUSSELS;
        case LOC_BERLIN:
            return LAT_BERLIN;
        case LOC_VIENNA:
            return LAT_VIENNA;
        case LOC_ROME:
            return LAT_ROME;
        case LOC_MADRID:
            return LAT_MADRID;
        case LOC_LISBON:
            return LAT_LISBON;
        case LOC_STOCKHOLM:
            return LAT_STOCKHOLM;
        case LOC_OSLO:
            return LAT_OSLO;
        case LOC_COPENHAGEN:
            return LAT_COPENHAGEN;
        case LOC_HELSINKI:
            return LAT_HELSINKI;
        case LOC_WARSAW:
            return LAT_WARSAW;
        case LOC_BUDAPEST:
            return LAT_BUDAPEST;
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
        case LOC_MANCHESTER:
            return LON_MANCHESTER;
        case LOC_EDINBURGH:
            return LON_EDINBURGH;
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
        case LOC_DUBLIN:
            return LON_DUBLIN;
        case LOC_PARIS:
            return LON_PARIS;
        case LOC_AMSTERDAM:
            return LON_AMSTERDAM;
        case LOC_BRUSSELS:
            return LON_BRUSSELS;
        case LOC_BERLIN:
            return LON_BERLIN;
        case LOC_VIENNA:
            return LON_VIENNA;
        case LOC_ROME:
            return LON_ROME;
        case LOC_MADRID:
            return LON_MADRID;
        case LOC_LISBON:
            return LON_LISBON;
        case LOC_STOCKHOLM:
            return LON_STOCKHOLM;
        case LOC_OSLO:
            return LON_OSLO;
        case LOC_COPENHAGEN:
            return LON_COPENHAGEN;
        case LOC_HELSINKI:
            return LON_HELSINKI;
        case LOC_WARSAW:
            return LON_WARSAW;
        case LOC_BUDAPEST:
            return LON_BUDAPEST;
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
        case LOC_MANCHESTER:
        case LOC_EDINBURGH:
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
        case LOC_DUBLIN:
            return "Europe/Dublin";
        case LOC_PARIS:
            return "Europe/Paris";
        case LOC_AMSTERDAM:
        case LOC_BRUSSELS:
            return "Europe/Amsterdam";
        case LOC_BERLIN:
            return "Europe/Berlin";
        case LOC_VIENNA:
            return "Europe/Vienna";
        case LOC_ROME:
            return "Europe/Rome";
        case LOC_MADRID:
            return "Europe/Madrid";
        case LOC_LISBON:
            return "Europe/Lisbon";
        case LOC_STOCKHOLM:
            return "Europe/Stockholm";
        case LOC_OSLO:
            return "Europe/Oslo";
        case LOC_COPENHAGEN:
            return "Europe/Copenhagen";
        case LOC_HELSINKI:
            return "Europe/Helsinki";
        case LOC_WARSAW:
            return "Europe/Warsaw";
        case LOC_BUDAPEST:
            return "Europe/Budapest";
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
        case LOC_MANCHESTER:
            return "Manchester";
        case LOC_EDINBURGH:
            return "Edinburgh";
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
        case LOC_DUBLIN:
            return "Dublin";
        case LOC_PARIS:
            return "Paris";
        case LOC_AMSTERDAM:
            return "Amsterdam";
        case LOC_BRUSSELS:
            return "Brussels";
        case LOC_BERLIN:
            return "Berlin";
        case LOC_VIENNA:
            return "Vienna";
        case LOC_ROME:
            return "Rome";
        case LOC_MADRID:
            return "Madrid";
        case LOC_LISBON:
            return "Lisbon";
        case LOC_STOCKHOLM:
            return "Stockholm";
        case LOC_OSLO:
            return "Oslo";
        case LOC_COPENHAGEN:
            return "Copenhagen";
        case LOC_HELSINKI:
            return "Helsinki";
        case LOC_WARSAW:
            return "Warsaw";
        case LOC_BUDAPEST:
            return "Budapest";
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
