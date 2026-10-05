#include <raylib.h>
#include "ui.h"
#include "game.h"

// Functions
static void start(void);
static void loop(void);
static void stop(void);

static void start(void)
{
    // Initializes the main game window
    InitializeMainWindow();

    // Starts the main loop
    loop();
}

static void stop(void)
{
    // Closes the main window
    CloseWindow();
}

static void loop(void)
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

int main(void)
{
    // Start game
    start();

    // Cleanup
    stop();
    
    return 0;
}
