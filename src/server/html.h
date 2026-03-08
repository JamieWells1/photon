#ifndef HTML_H
#define HTML_H

#include <flash_memory.h>

const char* html_get_wifi_form(void);
const char* html_get_success_page(void);
const char* html_build_config_page(const device_config_t* config, const char* active_tab);
const char* html_get_portfolio_success(void);
const char* html_get_portfolio_error(void);

#endif  // HTML_H
