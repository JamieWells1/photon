#ifndef RESET_H
#define RESET_H

#include <input.h>
#include <matrix.h>

void reset_display(Matrix* mtrx);
void reset_detect_inputs(Button* btns, Rotator* rtr, Matrix* mtrx);
void reset_cleanup(void);
bool reset_is_ready(void);

#endif  // RESET_H
