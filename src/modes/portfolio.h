#ifndef PORTFOLIO_H
#define PORTFOLIO_H

#include <matrix.h>
#include <stdint.h>

void portfolio_display(Matrix* mtrx, int* offset);
void portfolio_next_metric(Matrix* mtrx);
void portfolio_prev_metric(Matrix* mtrx);
void portfolio_cleanup(void);

#endif  // PORTFOLIO_H
