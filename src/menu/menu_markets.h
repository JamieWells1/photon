#ifndef MENU_MARKETS_H
#define MENU_MARKETS_H

#include <input.h>
#include <matrix.h>

void menu_markets_start(Button* btns, Rotator* rtr, Matrix* mtrx);
void menu_markets_detect_inputs(Button* btns, Rotator* rtr, Matrix* mtrx);
void menu_markets_cleanup(void);

#endif  // MENU_MARKETS_H
