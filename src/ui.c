#include <raylib.h>
#include <stdbool.h>

#include "ui.h"
#include "game.h"

Color BuildColor(int r, int g, int b)
{
    return (Color){(unsigned char)r, (unsigned char)g, (unsigned char)b, 255};
}

void DrawGameMenu(void)
{
    const Rectangle startButton = {362.0f, 302.0f, 300.0f, 62.0f};
    const bool hovering = CheckCollisionPointRec(GetMousePosition(), startButton);
    const bool startPressed = hovering && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    const Color buttonColor = hovering ? (Color){73, 184, 119, 255} : (Color){42, 135, 171, 255};

    BeginDrawing();
    DrawRectangleGradientV(0, 0, GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT,
                           (Color){29, 52, 65, 255}, (Color){12, 24, 32, 255});
    DrawCircle(824, 109, 78.0f, Fade((Color){232, 171, 99, 255}, 0.13f));
    DrawCircle(824, 109, 54.0f, Fade((Color){232, 171, 99, 255}, 0.17f));

    DrawText("STEEL", 104, 76, 66, RAYWHITE);
    DrawText("MONSTERS", 104, 139, 66, (Color){226, 163, 100, 255});
    DrawText("TURN-BASED ARTILLERY DUEL", 108, 224, 19, (Color){170, 193, 198, 255});

    DrawRectangleRounded(startButton, 0.18f, 10, buttonColor);
    const char *buttonLabel = "START BATTLE";
    DrawText(buttonLabel, (GAME_WINDOW_WIDTH - MeasureText(buttonLabel, 24)) / 2,
             (int)startButton.y + 17, 24, RAYWHITE);

    DrawText("UP / DOWN  -  AIM       W / S  -  POWER", 306, 390, 18, (Color){205, 216, 212, 255});
    DrawText("A / D  -  MOVE       SPACE  -  FIRE       P  -  PAUSE", 226, 421, 18, (Color){205, 216, 212, 255});

    EndDrawing();

    if (startPressed) StartGame();
}

void InitializeMainWindow(void)
{
    InitWindow(GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT, WINDOW_TITLE);
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
}
