/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#ifndef WEATHER_H
#define WEATHER_H

#include <matrix.h>

int weather_current_hour_index(void);
int weather_hours_fetched(void);

void weather_display(Matrix* mtrx, int* hour_to_display);
void weather_cleanup(void);

#endif  // WEATHER_H
