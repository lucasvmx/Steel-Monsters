
#include <raylib.h>
#include "game.h"
#include "log.h"

// Predefined functions
static void DrawFrame();

// Global variables
static bool gameRunning = false;

bool isGameRunning()
{
    return gameRunning;
}

void StartGame()
{
    gameRunning = true;

    println("[INFO] Starting game");

    // Clean screen
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Run game

    // End drawing
    EndDrawing();
}

void RunGame()
{
    BeginDrawing();
    DrawFrame();
    EndDrawing();
}

static void DrawFrame()
{
    ClearBackground(RAYWHITE);
    DrawFPS(0, 0);

    // Read keys
    
}
