#ifndef UI_H
#define UI_H

#include <raylib.h>

/**
 * @brief Height of game window
 * 
 */
#define GAME_WINDOW_HEIGHT  500

/**
 * @brief Width of game window
 * 
 */
#define GAME_WINDOW_WIDTH   1024

/**
 * @brief Title of game window
 * 
 */
#define WINDOW_TITLE        "Steel Monsters"

/**
 * @brief Returns the capacity of x
 * 
 */
#define cap(x)  (sizeof(x)/sizeof(x[0]))

typedef struct Button {
    Rectangle rect;
    Color color;
    unsigned id;
} BUTTON;

extern void DrawGameMenu();
extern void InitializeMainWindow();
extern Color BuildColor(int r, int g, int b);

#endif
