/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#ifndef PORTFOLIO_H
#define PORTFOLIO_H

#include <matrix.h>
#include <stdint.h>

void portfolio_display(Matrix* mtrx, int* offset);
void portfolio_next_metric(Matrix* mtrx);
void portfolio_prev_metric(Matrix* mtrx);
bool portfolio_is_auto_scroll(void);
void portfolio_cleanup(void);

#endif  // PORTFOLIO_H
