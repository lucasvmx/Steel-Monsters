
#include <raylib.h>
#include <unistd.h>
#include "game.h"
#include "log.h"
#include "ui.h"
#include "tank.h"

// Predefined functions
static void DrawFrame();

// Global variables
static bool gameRunning = false;
static bool gamePaused = false;

static Image tankImg;
static Texture2D tankTexture;
static Sound shootSound;

bool isGameRunning()
{
    return gameRunning;
}

void StartGame()
{
    gameRunning = true;

    log_notice("Starting game");

    // Clean screen
    BeginDrawing();
    ClearBackground(RAYWHITE);
    // End drawing
    EndDrawing();

    // Start audio system
    InitAudioDevice();

    shootSound = LoadSound("sounds/cannon.wav");
    SetSoundVolume(shootSound, 0.2f);

    tankImg = LoadTankImage();
    tankTexture = LoadTextureFromImage(tankImg);

    // Run game
    RunGame();
}

void RunGame()
{
    BeginDrawing();
    DrawFrame();
    EndDrawing();
}

void StopGame()
{
    CloseAudioDevice();
}

static void DrawFrame()
{
    static int tank_x = 0, tank_y = 100;
    Rectangle ground, tank;
    static float elapsed = 0;
    const bool start_right = true;
    static bool moving_right = start_right;

    ClearBackground(RAYWHITE);
    DrawFPS(0, 0);

    if(gamePaused) {
        DrawText("GAME PAUSED", (GAME_WINDOW_WIDTH / 6) * 2, 
        (GAME_WINDOW_HEIGHT / 2) - (GAME_WINDOW_HEIGHT / 4), 50, BLUE);
    }

    elapsed += GetFrameTime();

    if(IsKeyDown(KEY_P) && (elapsed > 0.25)) {
        gamePaused = !gamePaused;

        if(gamePaused) {
            log_notice("Game paused: %lf", elapsed);
        } else {
            log_notice("Game unpaused");
        }

        elapsed = 0;
    }

    int tank_width = tankTexture.width, tank_height = tankTexture.height;

    // Draw earth
    const int ground_height = GAME_WINDOW_HEIGHT - (GAME_WINDOW_HEIGHT / 3);
    DrawRectangle(0, ground_height, GAME_WINDOW_WIDTH, ground_height, BROWN);

    ground.height = ground_height;
    ground.width = GAME_WINDOW_WIDTH;
    ground.x = 0;
    ground.y = ground_height;

    // Tank is on ground
    tank_y = ground.y - tank_height;

    // Draw tank
    DrawTank(tankTexture, tank_x, tank_y, BuildColor(0, 255, 180));
    
    tank.x = tank_x;
    tank.y = tank_y;
    tank.width = tank_width;
    tank.height = tank_height;

    if(!gamePaused) {
        if(IsKeyDown(KEY_D)) {
            // Move tank to the right
            if(tank_x < GAME_WINDOW_WIDTH) {
                
                tank_x += 1;

                // Flip image
                if(!moving_right) {

                    ImageFlipHorizontal(&tankImg);

                    moving_right = true;

                    tankTexture = LoadTextureFromImage(tankImg);
                }
            } else {
                log_notice("You can't move to the right anymore");
                tank_x = GAME_WINDOW_WIDTH - (tank_width + 10);
            }
        } else if(IsKeyDown(KEY_A)) {
            // Move to left
            if(tank_x > 0) {
                tank_x -= 1;

                // Rotate 180 deg
                if(moving_right) {
                    ImageFlipHorizontal(&tankImg);
                    moving_right = false;
                    tankTexture = LoadTextureFromImage(tankImg);
                }
            } else {
                log_notice("You can't move to the left anymore");
            }
        } else if(IsKeyDown(KEY_SPACE)) {
            // Shoot
            PlaySound(shootSound);
        }
    }
}
