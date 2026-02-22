#ifndef WEATHER_H
#define WEATHER_H

#include <menu.h>

int weather_current_hour_index();

void weather_display(SubMode sub_mode, Matrix* mtrx, int* hour_to_display);
void weather_cleanup(void);

#endif  // WEATHER_H
