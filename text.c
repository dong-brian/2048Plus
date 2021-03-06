#include <stdio.h>

#include "GBALib.h"
#include "Text.h"

void drawChar(int row, int col, u8 ch, u16 color) {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 6; c++) {
            if (fontdata_6x8[OFFSET(r, c, 6) + ch*48]) {
                setPixel(r + row, c + col, color);
            }
        }
    }
}

void drawString(int row, int col, char* str, u16 color) {
    while (*str) {
        drawChar(row, col, *str++, color);
        col += 6;
    }
}
