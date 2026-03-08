#ifndef MENU_MAIN_H
#define MENU_MAIN_H

#include <input.h>
#include <matrix.h>
#include <stdbool.h>

void menu_main_start(Button* btns, Rotator* rtr, Matrix* mtrx);
void menu_main_reset_states(Matrix* mtrx);
bool menu_main_in_submenu(void);
void menu_main_exit_submenu(Matrix* mtrx);

#endif  // MENU_MAIN_H
