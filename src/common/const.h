#ifndef CONST_H
#define CONST_H

// Debug mode
#define DEBUG_LOGS 1

// Matrix layout
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 8
#define NUM_PIXELS (MATRIX_WIDTH * MATRIX_HEIGHT)

// Matrix pixels
#define BRIGHTNESS_COEFFICIENT 0.5

// GPIO pins
#define BUTTON_LEFT_PIN 10  // GP10
#define BUTTON_RIGHT_PIN 5  // GP5

#define ROTATOR_CLK_PIN 18  // GP18
#define ROTATOR_DT_PIN 19   // GP19
#define ROTATOR_SW_PIN 20   // GP20

#define MATRIX_MATRIX_PIN 2  // GP2
#define MATRIX_DATA_FREQ 800000

#endif  // CONST_H
