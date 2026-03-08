#ifndef PORTFOLIO_H
#define PORTFOLIO_H

#include <matrix.h>
#include <stdint.h>

void portfolio_display(Matrix* mtrx, int* offset);
void portfolio_next_metric(void);
void portfolio_prev_metric(void);
void portfolio_cleanup(void);

#endif  // PORTFOLIO_H
