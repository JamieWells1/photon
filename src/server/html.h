/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#ifndef HTML_H
#define HTML_H

#include <flash_memory.h>

const char* html_get_wifi_form(void);
const char* html_get_success_page(void);
const char* html_build_config_page(const device_config_t* config, const char* active_tab);
const char* html_get_portfolio_success(void);
const char* html_get_portfolio_error(const char* reason);
const char* html_build_update_page(bool update_available, const char* remote_version);

const char* html_build_ticker_checking_page(const char* ticker);
const char* html_build_ticker_error_page(const char* ticker);
const char* html_get_ticker_still_checking(void);

#endif  // HTML_H
