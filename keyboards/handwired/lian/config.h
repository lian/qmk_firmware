#pragma once

//#include "config_common.h"

/* USB Device descriptor parameter */
#define VENDOR_ID       0xFEED
#define PRODUCT_ID      0x6060
#define DEVICE_VER      0x0001
#define MANUFACTURER    lang.ink
#define PRODUCT         ortho4x12
#define DESCRIPTION     A compact ortholinear keyboard

/* key matrix size */
#define MATRIX_ROWS 4
#define MATRIX_COLS 12

// wiring
#define MATRIX_ROW_PINS_MCP \
    { B4, B5, B6, B7 }
#define MATRIX_COL_PINS_MCP \
    { A0, A1, A2, A3, A4, A5, A6, A7, B0, B1, B2, B3 }

/* COL2ROW, ROW2COL*/
#define DIODE_DIRECTION COL2ROW

#define DEBOUNCE 5

//#define DEBUG_MATRIX_SCAN_RATE
