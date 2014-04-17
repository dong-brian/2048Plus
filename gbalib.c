#include "GBALib.h"

u16* videoBuffer = (u16*) 0x6000000;

void setPixel(int row, int col, u16 color) {
    videoBuffer[OFFSET(row, col, 240)] = color;
}

void fillScreen(volatile u16 color) {
    DMA[3].src = &color;
    DMA[3].dst = videoBuffer;
    DMA[3].cnt = 38400 | DMA_SOURCE_FIXED | DMA_ON;
}

void clear() {
    fillScreen(BLACK);
}

void drawHorizontalLine(int row, int col, int length, volatile u16 color) {
    DMA[3].src = &color;
    DMA[3].dst = &videoBuffer[OFFSET(row, col, 240)];
    DMA[3].cnt = length | DMA_SOURCE_FIXED | DMA_ON;
}

void drawVerticalLine(int row, int col, int length, u16 color) {
    for (int r = 0; r < length; r++) {
        videoBuffer[OFFSET(row + r, col, 240)] = color;
    }
}

void drawRect(int row, int col, int width, int height, u16 color) {
    for (int r = 0; r < height; r++) {
        drawHorizontalLine(row + r, col, width, color);
    }
}

void drawHollowRect(int row, int col, int width, int height, u16 color) {
    drawHorizontalLine(row, col, width, color);
    drawHorizontalLine(row + height - 1, col, width, color);
    drawVerticalLine(row + 1, col, height - 2, color);
    drawVerticalLine(row + 1, col + width - 1, height - 2, color);
}

void copyRowHorizontal(int row, int col, int length, int shift) {
    DMA[3].src = &videoBuffer[OFFSET(row, col, 240)];
    DMA[3].dst = &videoBuffer[OFFSET(row, col + shift, 240)];
    DMA[3].cnt = 0;
    if (shift > 0) {
	DMA[3].src = &videoBuffer[OFFSET(row, col, 240) + length - 1];
	DMA[3].dst = &videoBuffer[OFFSET(row, col + shift, 240) + length - 1];
        DMA[3].cnt = DMA_SOURCE_DECREMENT | DMA_DESTINATION_DECREMENT;
    }
    DMA[3].cnt = DMA[3].cnt | length | DMA_ON;
}

void copyRowVertical(int row, int col, int length, int shift) {
    DMA[3].src = &videoBuffer[OFFSET(row, col, 240)];
    DMA[3].dst = &videoBuffer[OFFSET(row + shift, col, 240)];
    DMA[3].cnt = length | DMA_ON;
}

void shiftRowHorizontal(int row, int col, int length, int shift) {
    copyRowHorizontal(row, col, length, shift);
    if (shift >= 0) {
        drawHorizontalLine(row, col, shift, BLACK);
    } else {
        drawHorizontalLine(row, col + length + shift, -shift, BLACK);
    }
}

void shiftRowVertical(int row, int col, int length, int shift) {
    copyRowVertical(row, col, length, shift);
    drawHorizontalLine(row, col, length, BLACK);
}

void shiftRectHorizontal(int row, int col, int width, int height, int shift) {
    for (int r = 0; r < height; r++) {
        shiftRowHorizontal(row + r, col, width, shift);
    }
}

void shiftRectVertical(int row, int col, int width, int height, int shift) {
    if (shift > 0) {
        for (int r = height - 1; r >= 0; r--) {
            shiftRowVertical(row + r, col, width, shift);
        }
    } else {
        for (int r = 0; r < height; r++) {
            shiftRowVertical(row + r, col, width, shift);
        }
    }
}

void drawImage(const unsigned short *img) {
    DMA[3].src = img;
    DMA[3].dst = videoBuffer;
    DMA[3].cnt = 38400 | DMA_ON;
}

void drawImageAt(int row, int col, int width, int height, const unsigned short *img) {
    for (int r = 0; r < height; r++) {
        DMA[3].src = img;
        DMA[3].dst = &videoBuffer[OFFSET(row + r, col, 240)];
        DMA[3].cnt = width | DMA_ON;
    }
}

void blit(int row, int col, int width, int height, int x, int y, int imgWidth, const unsigned short *img) {
    for (int r = 0; r < height; r++) {
        DMA[3].src = &img[OFFSET(y + r, x, imgWidth)];
        DMA[3].dst = &videoBuffer[OFFSET(row + r, col, 240)];
        DMA[3].cnt = width | DMA_ON;
    }
}

void waitForVblank() {
    while (SCANLINECOUNTER > 160);
    while (SCANLINECOUNTER < 160);
}
