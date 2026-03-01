#include <utils.h>

uint64_t get_current_time_ms(void) { return to_ms_since_boot(get_absolute_time()); }
