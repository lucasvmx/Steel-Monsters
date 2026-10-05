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

void DrawGameMenu(void);
void InitializeMainWindow(void);
Color BuildColor(int r, int g, int b);

#endif
