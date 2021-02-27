#include <raylib.h>
#include <stdio.h>
#include <string.h>

#include "ui.h"
#include "game.h"

static Color color;

static Color BuildColor(int r, int g, int b) 
{
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = 255;

    return color;
}

void DrawGameMenu()
{
    int rx, ry, rw, rh;
    const char *startGameText = "START GAME";

    BeginDrawing();
    ClearBackground(RAYWHITE);

    Color menuColor = BuildColor(0, 191, 255);
    Color textColor = BuildColor(255, 255, 255);

    // Calculates rectangle vertical alignment
    rx = GAME_WINDOW_WIDTH / 4;
    ry = GAME_WINDOW_HEIGHT / 2;
    rw = 2 * rx;
    rh = 50;

    DrawRectangle(rx, ry, rw, rh, menuColor);
    
    // Draw text in center of previous rectangle
    DrawText("START GAME", rx + 5, ry, rh, textColor);

    EndDrawing();

    Vector2 mousePos = GetMousePosition();

    Rectangle rect = {
        .width = rw,
        .height = rh,
        .x = rx,
        .y = ry
    };

    if(CheckCollisionPointRec(mousePos, rect)) {
        if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            // Game can be started
            StartGame();
        }
    }
}

void InitializeMainWindow()
{
    int width, height;

    width = GetScreenWidth();
    height = GetScreenHeight();

    // Initialize game window
    InitWindow(GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT, WINDOW_TITLE);

    // Configure FPS
    SetTargetFPS(30);
}
