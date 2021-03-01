#include <raylib.h>
#include <stdio.h>
#include "ui.h"
#include "game.h"
#include "log.h"
#include "../build/config.h"

// Functions
static void start();
static void loop();
static void stop();

static void start()
{
    log_notice("Starting %s", PACKAGE_STRING);

    // Initializes the main game window
    InitializeMainWindow();

    // Starts the main loop
    loop();
}

static void stop()
{
    // Closes the main window
    CloseWindow();
}

static void loop()
{
    // Draw menus
    while(!WindowShouldClose())
    {
        if(!isGameRunning()) {
            DrawGameMenu();
        } else {
            RunGame();
        }
    }

    // Cleanup resources
    StopGame();
}

int main()
{
    // Start game
    start();

    // Cleanup
    stop();
    
    return 0;
}