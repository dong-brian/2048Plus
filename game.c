#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "GBALib.h"
#include "Text.h"
#include "KongouSisters.h"
#include "Haruna.h"
#include "Hiei.h"
#include "Kirishima.h"
#include "Kongou.h"

//integer percent for which 4's will spawn
#define SPAWN_PERCENT 20
#define LEFT_MARGIN 30
#define RIGHT_MARGIN 179
#define TOP_MARGIN 9
#define BLOCK_SIZE 13
#define BLOCK_SPEED 2
#define BLOCK_NUM_MOVES 7
#define SCORE_MAX_CHAR 10
#define BG_COLOR BLACK

typedef enum {
    RANDOM,
    FIXED
} Style;

typedef enum {
    CHANGE_SIZE,
    CHANGE_MODE
} State_Settings_Menu;

typedef enum {
    SPLASH,
    INSTRUCTIONS,
    SETTINGS,
    GAME,
    GAME_OVER
} State_Game;

typedef enum {
    WIN,
    LOSE
} Outcome;

typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT
} Direction;

typedef struct {
    int seed;
    int size;
    Style mode;
} Info;

typedef struct {
    Outcome outcome;
    int score;
} Result;

typedef struct {
    int num;
    u16 color;
} Block;

extern u16* videoBuffer;

const int colors[] = {
    RED,
    GREEN,
    BLUE,
    CYAN,
    MAGENTA,
    YELLOW
};
const int sizeColors = sizeof(colors)/sizeof(colors[0]);

int splash(Info* i);
void settings(Info* i);
void instructions();
Result game(int seed, int size, Style mode);
void gameOver(Outcome outcome, int score);
void drawSplash();
void drawBlockCoor(int x, int y, Block b);
void drawBlock(Block* blocks, int size, int index);
void drawBlocks(Block* blocks, int size);
void drawMovingBlocks(int row, int col, int size, Direction d, int spaces);
void drawBoard(int size);
void drawGameText();
void updateScoreDisplay(int score);
int spawnBlock(Block* blocks, int blocksLeft, Style mode);
int moveBlock(int row, int col, Block* blocks, int size, Style mode, Direction d, int* score, int* blocksLeft, Outcome* outcome);
int moveBlocks(Block* blocks, int size, Style mode, Direction d, int* score, int* blocksLeft, Outcome* outcome);
int hasMoves(Block* blocks, int size);
void initializeBlocks(Block* blocks, int size);
u16 getColor(int i, Style mode);

int main() {
    REG_DISPCTL = MODE3 | BG2_ENABLE;
    State_Game state = SPLASH;
    Result result = {LOSE, 0};
    Info ii = {0, 4, FIXED};
    Info* i = &ii;
    int button = 0;
    while (1) {
        switch (state) {
            case SPLASH:
                button = splash(i);
                switch (button) {
                    case BUTTON_START:
                        state = GAME;
                        break;
                    case BUTTON_A:
                        state = INSTRUCTIONS;
                        break;
                    case BUTTON_SELECT:
                        state = SETTINGS;
                        break;
                }
                break;
            case INSTRUCTIONS:
                instructions();
                state = GAME;
                break;
            case SETTINGS:
                settings(i);
                state = SPLASH;
                break;
            case GAME:
                result = game(i->seed, i->size, i->mode);
                state = GAME_OVER;
                break;
            case GAME_OVER:
                gameOver(result.outcome, result.score);
                state = SPLASH;
                break;
        }
    }
}

/* 
 * Draw the splash screen and perform the appropriate logic.
 * Pressing Start begins the game.
 * Pressing Select opens the settings menu.
 * Pressing A opens the instructions.
 * 
 * @return  the button pressed to exit the splash screen
 */
int splash(Info* i) {
    int result = 0;
    waitForVblank();
    drawSplash();
    while (!KEY_DOWN_NOW(BUTTON_START) && !KEY_DOWN_NOW(BUTTON_A)
            && !KEY_DOWN_NOW(BUTTON_SELECT)) {
        i->seed++;
    }
    if (KEY_DOWN_NOW(BUTTON_START)) {
        result = BUTTON_START;
    } else if (KEY_DOWN_NOW(BUTTON_A)) {
        result = BUTTON_A;
    } else if (KEY_DOWN_NOW(BUTTON_SELECT)) {
        result = BUTTON_SELECT;
    }
    while (KEY_DOWN_NOW(BUTTON_START) || KEY_DOWN_NOW(BUTTON_A)
            || KEY_DOWN_NOW(BUTTON_SELECT));
    return result;
}

/* 
 * 
 */
void settings(Info* i) {
    State_Settings_Menu state = CHANGE_SIZE;
    Info jj = {0, i->size, i->mode};
    Info* j = &jj;
    char buffer[9];
    char* modeString = "";
    char* oldModeString = "";
    sprintf(buffer, "Size: %d", j->size);
    switch (j->mode) {
        case FIXED:
            modeString = "Color Mode: Fixed";
            break;
        case RANDOM:
            modeString = "Color Mode: Random";
            break;
    }
    waitForVblank();
    fillScreen(BLACK);
    waitForVblank();
    drawString(30, 10, "Use Up and Down to choose a setting.", YELLOW);
    drawString(50, 10, "Use Left and Right to modify.", YELLOW);
    drawString(70, 30, buffer, YELLOW);
    drawString(90, 30, modeString, YELLOW);
    drawString(110, 10, "Press Start to save changes.", YELLOW);
    drawString(130, 10, "Press B to exit without saving.", YELLOW);
    drawString(70, 10, ">>", GREEN);
    while (!KEY_DOWN_NOW(BUTTON_START) && !KEY_DOWN_NOW(BUTTON_B)) {
        if (KEY_DOWN_NOW(BUTTON_UP)) {
            switch (state) {
                case CHANGE_SIZE:
                    state = CHANGE_MODE;
                    waitForVblank();
                    drawString(70, 10, ">>", BLACK);
                    drawString(90, 10, ">>", GREEN);
                    break;
                case CHANGE_MODE:
                    state = CHANGE_SIZE;
                    waitForVblank();
                    drawString(90, 10, ">>", BLACK);
                    drawString(70, 10, ">>", GREEN);
                    break;
            }
            while (KEY_DOWN_NOW(BUTTON_UP));
        }
        if (KEY_DOWN_NOW(BUTTON_DOWN)) {
            switch (state) {
                case CHANGE_SIZE:
                    state = CHANGE_MODE;
                    waitForVblank();
                    drawString(70, 10, ">>", BLACK);
                    drawString(90, 10, ">>", GREEN);
                    break;
                case CHANGE_MODE:
                    state = CHANGE_SIZE;
                    waitForVblank();
                    drawString(90, 10, ">>", BLACK);
                    drawString(70, 10, ">>", GREEN);
                    break;
            }
            while (KEY_DOWN_NOW(BUTTON_DOWN));
        }
        if (KEY_DOWN_NOW(BUTTON_RIGHT)) {
            switch (state) {
                case CHANGE_SIZE:
                    if (j->size < 10) {
                        j->size++;
                    } else {
                        j->size = 2;
                    }
                    sprintf(buffer, "Size: %d", j->size);
                    waitForVblank();
                    drawRect(70, 30, 6*8, 8, BLACK);
                    drawString(70, 30, buffer, YELLOW);
                    break;
                case CHANGE_MODE:
                    switch (j->mode) {
                        case FIXED:
                            j->mode = RANDOM;
                            oldModeString = modeString;
                            modeString = "Color Mode: Random";
                            break;
                        case RANDOM:
                            j->mode = FIXED;
                            oldModeString = modeString;
                            modeString = "Color Mode: Fixed";
                            break;
                    }
                    waitForVblank();
                    drawString(90, 30, oldModeString, BLACK);
                    drawString(90, 30, modeString, YELLOW);
                    break;
            }
            while (KEY_DOWN_NOW(BUTTON_RIGHT));
        }
        if (KEY_DOWN_NOW(BUTTON_LEFT)) {
            switch (state) {
                case CHANGE_SIZE:
                    if (j->size > 2) {
                        j->size--;
                    } else {
                        j->size = 10;
                    }
                    sprintf(buffer, "Size: %d", j->size);
                    waitForVblank();
                    drawRect(70, 30, 6*8, 8, BLACK);
                    drawString(70, 30, buffer, YELLOW);
                    break;
                case CHANGE_MODE:
                    switch (j->mode) {
                        case FIXED:
                            j->mode = RANDOM;
                            oldModeString = modeString;
                            modeString = "Color Mode: Random";
                            break;
                        case RANDOM:
                            j->mode = FIXED;
                            oldModeString = modeString;
                            modeString = "Color Mode: Fixed";
                            break;
                    }
                    waitForVblank();
                    drawString(90, 30, oldModeString, BLACK);
                    drawString(90, 30, modeString, YELLOW);
                    break;
            }
            while (KEY_DOWN_NOW(BUTTON_LEFT));
        }
        if (KEY_DOWN_NOW(BUTTON_START)) {
            i->size = j->size;
            i->mode = j->mode;
        }
        i->seed++;
    }
    while (KEY_DOWN_NOW(BUTTON_START) || KEY_DOWN_NOW(BUTTON_B));
}

/* 
 * Press Left or Right to change pages.
 * Press Start to begin
 */
void instructions() {
    waitForVblank();
    fillScreen(BLACK);
    drawString(30, 10, "D-Pad to move blocks.", YELLOW);
    drawString(50, 10, "If two blocks with the same", YELLOW);
    drawString(70, 10, "number hit, they will merge!", YELLOW);
    drawString(130, 10, "Press Start to begin.", YELLOW);
    while (!KEY_DOWN_NOW(BUTTON_START));
    while (KEY_DOWN_NOW(BUTTON_START));
}

/* 
 * 
 */
Result game(int seed, int size, Style mode) {
    Result result = {LOSE, 0};
    srand(seed);
    Block blocks[size*size];
    initializeBlocks(blocks, size);
    int blocksLeft = size*size;
    int index[2];
    index[0] = spawnBlock(blocks, blocksLeft--, mode);
    index[1] = spawnBlock(blocks, blocksLeft--, mode);
    result.score += 1<<blocks[index[0]].num;
    result.score += 1<<blocks[index[1]].num;
    waitForVblank();
    fillScreen(BLACK);
    drawBoard(size);
    drawGameText();
    updateScoreDisplay(result.score);
    drawBlock(blocks, size, index[0]);
    drawBlock(blocks, size, index[1]);
    int spawn = FALSE;
    int quit = FALSE;
    while (!quit) {
        if (KEY_DOWN_NOW(BUTTON_UP)) {
            if (blocksLeft && moveBlocks(blocks, size, mode, UP, &result.score, &blocksLeft, &result.outcome)) {
                spawn = TRUE;
            }
            while (KEY_DOWN_NOW(BUTTON_UP));
        }
        if (KEY_DOWN_NOW(BUTTON_DOWN)) {
            if (blocksLeft && moveBlocks(blocks, size, mode, DOWN, &result.score, &blocksLeft, &result.outcome)) {
                spawn = TRUE;
            }
            while (KEY_DOWN_NOW(BUTTON_DOWN));
        }
        if (KEY_DOWN_NOW(BUTTON_LEFT)) {
            if (blocksLeft && moveBlocks(blocks, size, mode, LEFT, &result.score, &blocksLeft, &result.outcome)) {
                spawn = TRUE;
            }
            while (KEY_DOWN_NOW(BUTTON_LEFT));
        }
        if (KEY_DOWN_NOW(BUTTON_RIGHT)) {
            if (blocksLeft && moveBlocks(blocks, size, mode, RIGHT, &result.score, &blocksLeft, &result.outcome)) {
                spawn = TRUE;
            }
            while (KEY_DOWN_NOW(BUTTON_RIGHT));
        }
        if (spawn) {
            index[0] = spawnBlock(blocks, blocksLeft--, mode);
            result.score += 1<<blocks[index[0]].num;
            waitForVblank();
            drawBlock(blocks, size, index[0]);
            updateScoreDisplay(result.score);
            quit = !hasMoves(blocks, size);
            spawn = FALSE;
        }
        if (KEY_DOWN_NOW(BUTTON_B)) {
            quit = TRUE;
            while (KEY_DOWN_NOW(BUTTON_B));
        }
    }
    //put a while loop here to check if game ends properly
    return result;
}

/* 
 * 
 */
void gameOver(Outcome outcome, int score) {
    char* s = "";
    char buffer[39];
    switch (outcome) {
        case WIN:
            s = "You win! :D";
            break;
        case LOSE:
            s = "You lost. :(";
            break;
    }
    sprintf(buffer, "Score: %d", score);
    waitForVblank();
    fillScreen(BLACK);
    waitForVblank();
    drawString(30, 10, "GAME OVER", RED);
    drawString(50, 10, s, YELLOW);
    drawString(70, 10, buffer, YELLOW);
    drawString(90, 10, "Press Start to play again.", YELLOW);
    while (!KEY_DOWN_NOW(BUTTON_START));
    while (KEY_DOWN_NOW(BUTTON_START));
}

/* 
 * 
 */
void drawSplash() {
    drawImage(KongouSistersBitmap);
    waitForVblank();
    drawString(70, 10, "2048+", YELLOW);
    drawString(90, 10, "Press Start to begin.", YELLOW);
    drawString(110, 10, "Press Select to change settings.", YELLOW);
    drawString(130, 10, "Press A for how to play.", YELLOW);
}

/* 
 * 
 */
void drawBlockCoor(int row, int col, Block b) {
    int r = TOP_MARGIN + (BLOCK_SIZE + 1)*row + 1;
    int c = LEFT_MARGIN + (BLOCK_SIZE + 1)*col + 1;
    if (b.num) {
        u16 color = b.color;
        char buffer[3];
        sprintf(buffer, "%2d",  b.num);
        drawRect(r, c, BLOCK_SIZE, BLOCK_SIZE, color);
        drawString(r + 3, c, buffer, BLACK);
    } else {
        drawRect(r, c, BLOCK_SIZE, BLOCK_SIZE, BLACK);
    }
}

/* 
 * 
 */
void drawBlock(Block* blocks, int size, int index) {
    drawBlockCoor(index/size, index%size, blocks[index]);
}

/* 
 * 
 */
void drawBlocks(Block* blocks, int size) {
    for (int i = 0; i < size*size; i++) {
        drawBlock(blocks, size, i);
    }
}

/* 
 * 
 */
void drawMovingBlocks(int row, int col, int size, Direction d, int spaces) {
    int r = TOP_MARGIN + (BLOCK_SIZE + 1)*row + 1;
    int c = LEFT_MARGIN + (BLOCK_SIZE + 1)*col + 1;
    int length = 0;
    switch (d) {
        case UP:
            length = (size - row)*(BLOCK_SIZE + 1) - 1;
            for (int i = 0; i < spaces; i++) {
                for (int j = 0; j < BLOCK_NUM_MOVES; j++) {
                    shiftRectVertical(r + BLOCK_SPEED, c, BLOCK_SIZE, length, -BLOCK_SPEED);
                }
                drawHorizontalLine(r + length, c, BLOCK_SIZE, WHITE);
            }
            break;
        case DOWN:
            r = TOP_MARGIN + 1;
            length = (row + 1)*(BLOCK_SIZE + 1) - 1;
            for (int i = 0; i < spaces; i++) {
                for (int j = 0; j < BLOCK_NUM_MOVES; j++) {
                    shiftRectVertical(r, c, BLOCK_SIZE, length - BLOCK_SPEED, BLOCK_SPEED);
                }
                drawHorizontalLine(r + BLOCK_SIZE, c, BLOCK_SIZE, WHITE);
            }
            break;
        case LEFT:
            length = (size - col)*(BLOCK_SIZE + 1) - 1;
            for (int i = 0; i < spaces; i++) {
                for (int j = 0; j < BLOCK_NUM_MOVES; j++) {
                    shiftRectHorizontal(r, c + BLOCK_SPEED, length, BLOCK_SIZE, -BLOCK_SPEED);
                }
                drawVerticalLine(r, c + length, BLOCK_SIZE, WHITE);
            }
            break;
        case RIGHT:
            c = LEFT_MARGIN + 1;
            length = (col + 1)*(BLOCK_SIZE + 1) - 1;
            for (int i = 0; i < spaces; i++) {
                for (int j = 0; j < BLOCK_NUM_MOVES; j++) {
                    shiftRectHorizontal(r, c, length - BLOCK_SPEED, BLOCK_SIZE, BLOCK_SPEED);
                }
                drawVerticalLine(r, c + BLOCK_SIZE, BLOCK_SIZE, WHITE);
            }
            break;
    }
}

/* 
 * 
 */
void drawBoard(int size) {
    int length = (BLOCK_SIZE + 1)*size + 1;
    for (int i = 0; i <= size; i++) {
        drawHorizontalLine(TOP_MARGIN + (BLOCK_SIZE + 1)*i, LEFT_MARGIN, length, WHITE);
        drawVerticalLine(TOP_MARGIN, LEFT_MARGIN + (BLOCK_SIZE + 1)*i, length, WHITE);
    }
}

/* 
 * 
 */
void drawGameText() {
    drawString(30, RIGHT_MARGIN, "Score:", YELLOW);
    drawString(110, RIGHT_MARGIN, "Press B", YELLOW);
    drawString(130, RIGHT_MARGIN, "to quit.", YELLOW);
}

/* 
 * 
 */
void updateScoreDisplay(volatile int score) {
    drawRect(50, RIGHT_MARGIN, 6*SCORE_MAX_CHAR, 8, BLACK);
    char buffer[5];
    char buffer2[5];
    sprintf(buffer2, "%d", SCORE_MAX_CHAR);
    strcpy(buffer, "%");
    strcat(buffer, buffer2);
    strcat(buffer, "d");
    sprintf(buffer2, buffer, score);
    drawString(50, RIGHT_MARGIN, buffer2, YELLOW);
}

/* 
 * returns index of spawned block
 */
int spawnBlock(Block* blocks, int blocksLeft, Style mode) {
    if (blocksLeft < 1) {
        return 0;//dummy
    } else {
        int i = 0;
        while (blocks[i].num) {
            i++;
        }
        int index = rand()%blocksLeft;
        for (int j = 0; j < index; j++) {
            while (blocks[i].num) {
                i++;
            }
            i++;
        }
        while (blocks[i].num) {
            i++;
        }
        blocks[i].num = 1 + (rand()%100 < SPAWN_PERCENT);
        blocks[i].color = getColor(blocks[i].num, mode);\
        return i;
    }
}

/* 
 * TRUE if a move was made
 */
int moveBlock(int row, int col, Block* blocks, int size, Style mode, Direction d, int* score, int* blocksLeft, Outcome* outcome) {
    int result = FALSE;
    int count = 0;
    int found = FALSE;
    Block* b = blocks;
    switch (d) {
        case UP:
            if (row < size - 1) {
                while ((count < size - row) && !blocks[OFFSET(row + count, col, size)].num) {
                    count++;
                }
                if (count < size - row) {
                    if (count) {
                        for (int r = row; r < size; r++) {
                            if (r + count >= size) {
                                b = &blocks[OFFSET(r, col, size)];
                                b->num = 0;
                                b->color = BLACK;
                            } else {
                                blocks[OFFSET(r, col, size)] = blocks[OFFSET(r + count, col, size)];
                            }
                        }
                        drawMovingBlocks(row, col, size, UP, count);
                        result = TRUE;
                    }
                    b = &blocks[OFFSET(row, col, size)];
                    found = FALSE;
                    for (int r = row + 1; r < size && !found; r++) {
                        if (blocks[OFFSET(r, col, size)].num) {
                            found = r;
                        }
                    }
                    if (found) {
                        if (b->num == blocks[OFFSET(found, col, size)].num) {
                            (b->num)++;
                            b->color = getColor(b->num, mode);
                            if (b->num == 11) {
                                *outcome = WIN;
                            }
                            *score += + (1<<b->num);
                            (*blocksLeft)++;
                            drawBlock(blocks, size, OFFSET(row, col, size));
                            b = &blocks[OFFSET(found, col, size)];
                            b->num = 0;
                            b->color = BLACK;
                            drawBlock(blocks, size, OFFSET(found, col, size));
                        }
                        result = TRUE;
                    }
                }
            }
            break;
        case DOWN:
            if (row > 0) {
                while ((count <= row) && !blocks[OFFSET(row - count, col, size)].num) {
                    count++;
                }
                if (count <= row) {
                    if (count) {
                        for (int r = row; r >= 0; r--) {
                            if (r - count < 0) {
                                b = &blocks[OFFSET(r, col, size)];
                                b->num = 0;
                                b->color = BLACK;
                            } else {
                                blocks[OFFSET(r, col, size)] = blocks[OFFSET(r - count, col, size)];
                            }
                        }
                        drawMovingBlocks(row, col, size, DOWN, count);
                        result = TRUE;
                    }
                    b = &blocks[OFFSET(row, col, size)];
                    found = size;
                    for (int r = row - 1; r >= 0 && found == size; r--) {
                        if (blocks[OFFSET(r, col, size)].num) {
                            found = r;
                        }
                    }
                    if (found != size) {
                        if (b->num == blocks[OFFSET(found, col, size)].num) {
                            (b->num)++;
                            b->color = getColor(b->num, mode);
                            if (b->num == 11) {
                                *outcome = WIN;
                            }
                            *score += (1<<b->num);
                            (*blocksLeft)++;
                            drawBlock(blocks, size, OFFSET(row, col, size));
                            b = &blocks[OFFSET(found, col, size)];
                            b->num = 0;
                            b->color = BLACK;
                            drawBlock(blocks, size, OFFSET(found, col, size));
                        }
                        result = TRUE;
                    }
                }
            }
            break;
        case LEFT:
            if (col < size - 1) {
                while ((count < size - col) && !blocks[OFFSET(row, col + count, size)].num) {
                    count++;
                }
                if (count < size - col) {
                    if (count) {
                        for (int c = col; c < size; c++) {
                            if (c + count >= size) {
                                b = &blocks[OFFSET(row, c, size)];
                                b->num = 0;
                                b->color = BLACK;
                            } else {
                                blocks[OFFSET(row, c, size)] = blocks[OFFSET(row, c + count, size)];
                            }
                        }
                        drawMovingBlocks(row, col, size, LEFT, count);
                        result = TRUE;
                    }
                    b = &blocks[OFFSET(row, col, size)];
                    found = FALSE;
                    for (int c = col + 1; c < size && !found; c++) {
                        if (blocks[OFFSET(row, c, size)].num) {
                            found = c;
                        }
                    }
                    if (found) {
                        if (b->num == blocks[OFFSET(row, found, size)].num) {
                            (b->num)++;
                            b->color = getColor(b->num, mode);
                            if (b->num == 11) {
                                *outcome = WIN;
                            }
                            *score += + (1<<b->num);
                            (*blocksLeft)++;
                            drawBlock(blocks, size, OFFSET(row, col, size));
                            b = &blocks[OFFSET(row, found, size)];
                            b->num = 0;
                            b->color = BLACK;
                            drawBlock(blocks, size, OFFSET(row, found, size));
                        }
                        result = TRUE;
                    }
                }
            }
            break;
        case RIGHT:
            if (col > 0) {
                while ((count <= col) && !blocks[OFFSET(row, col - count, size)].num) {
                    count++;
                }
                if (count <= col) {
                    if (count) {
                        for (int c = col; c >= 0; c--) {
                            if (c - count < 0) {
                                b = &blocks[OFFSET(row, c, size)];
                                b->num = 0;
                                b->color = BLACK;
                            } else {
                                blocks[OFFSET(row, c, size)] = blocks[OFFSET(row, c - count, size)];
                            }
                        }
                        drawMovingBlocks(row, col, size, RIGHT, count);
                        result = TRUE;
                    }
                    b = &blocks[OFFSET(row, col, size)];
                    found = size;
                    for (int c = col - 1; c >= 0 && found == size; c--) {
                        if (blocks[OFFSET(row, c, size)].num) {
                            found = c;
                        }
                    }
                    if (found != size) {
                        if (b->num == blocks[OFFSET(row, found, size)].num) {
                            (b->num)++;
                            b->color = getColor(b->num, mode);
                            if (b->num == 11) {
                                *outcome = WIN;
                            }
                            *score += (1<<b->num);
                            (*blocksLeft)++;
                            drawBlock(blocks, size, OFFSET(row, col, size));
                            b = &blocks[OFFSET(row, found, size)];
                            b->num = 0;
                            b->color = BLACK;
                            drawBlock(blocks, size, OFFSET(row, found, size));
                        }
                        result = TRUE;
                    }
                }
            }
            break;
    }
    return result;
}

/* 
 * returns WIN if a 2048 block was created
 */
int moveBlocks(Block* blocks, int size, Style mode, Direction d, int* score, int* blocksLeft, Outcome* outcome) {
    int result = LOSE;
    switch (d) {
        case UP:
            for (int r = 0; r < size - 1; r++) {
                for (int c = 0; c < size; c++) {
                    result = result || moveBlock(r, c, blocks, size, mode, UP, score, blocksLeft, outcome);
                }
            }
            break;
        case DOWN:
            for (int r = size - 1; r >= 1; r--) {
                for (int c = 0; c < size; c++) {
                    result = result || moveBlock(r, c, blocks, size, mode, DOWN, score, blocksLeft, outcome);
                }
            }
            break;
        case LEFT:
            for (int c = 0; c < size - 1; c++) {
                for (int r = 0; r < size; r++) {
                    result = result || moveBlock(r, c, blocks, size, mode, LEFT, score, blocksLeft, outcome);
                }
            }
            break;
        case RIGHT:
            for (int c = size - 1; c >= 1; c--) {
                for (int r = 0; r < size; r++) {
                    result = result || moveBlock(r, c, blocks, size, mode, RIGHT, score, blocksLeft, outcome);
                }
            }
            break;
    }
    return result;
}

/* Returns TRUE if there are available moves, otherwise returns FALSE.
 * 
 * @return  TRUE if there are available moves, FALSE otherwise
 */
int hasMoves(Block* blocks, int size) {
    int limit = size - 1;
    int x = 0;
    for (int r = 0; r < size; r++) {
        for (int c = 0; c < size; c++) {
            x = blocks[OFFSET(r, c, size)].num;
            if (!x) {
                return TRUE;
            } else if (r != limit) {
                if (x == blocks[(r + 1)*size + c].num) {
                    return TRUE;
                }
            } else if (c != limit) {
                if (x == blocks[r*size + c + 1].num) {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

void initializeBlocks(Block* blocks, int size) {
    for (int i = 0; i < size*size; i++) {
        blocks[i].num = 0;
        blocks[i].color = BLACK;
    }
}

u16 getColor(int i, Style mode) {
    u16 color = BLACK;    
    switch (mode) {
        case FIXED:
                color = colors[i%sizeColors];
            break;
        case RANDOM:
                color = colors[rand()%sizeColors];
            break;
    }
    return color;
}
