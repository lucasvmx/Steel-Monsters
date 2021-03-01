#include <raylib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "ui.h"
#include "game.h"
#include "log.h"

#define ID_BUTTON_START 1

// Global variable
static Color color;

Color BuildColor(int r, int g, int b) 
{
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = 255;

    return color;
}

static void processMenuEvents()
{

}

static void setButtonID(BUTTON *btn, unsigned ID)
{
    btn->id = ID;
}

static void DrawButton(const char *text, int x, int y, int width, int height, Color color, Color textColor, BUTTON *btn)
{    
    DrawRectangle(x, y, width, height, color);
    DrawText(text, x + (width / 6), y, height, textColor);

    if(btn != NULL)
    {
        btn->color = color;
        btn->rect.width = width;
        btn->rect.height = height;
        btn->rect.x = x;
        btn->rect.y = y;
        btn->id = 0;
    } else {
        log_error("Null pointer to button detected");
    }
}

static void DrawButtons(BUTTON *buttons)
{
    int rx, ry, rw, rh;
    BUTTON buttonStart;
    Color buttonColor, textColor;

    // Calculates rectangle vertical alignment
    rx = GAME_WINDOW_WIDTH / 4;
    ry = GAME_WINDOW_HEIGHT / 2;
    rw = 2 * rx;
    rh = 50;

    // Select colors
    buttonColor = BuildColor(0, 191, 255);
    textColor = BuildColor(255, 255, 255);

    // Draw game start button
    DrawButton("START GAME", rx, ry, rw, rh, buttonColor, textColor, &buttonStart);
    setButtonID(&buttonStart, ID_BUTTON_START);
    buttons[0] = buttonStart;

    // Change button color when mouse is hover
    if(CheckCollisionPointRec(GetMousePosition(), buttonStart.rect)) {
        buttonColor = BuildColor(50, 170, 0);
        DrawButton("START GAME", rx, ry, rw, rh, buttonColor, textColor, &buttonStart);
    }

    EndDrawing();
}

void DrawGameMenu()
{
    BUTTON buttons[1];

    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    // Draw buttons
    DrawButtons(buttons);

    // Process events
    for(unsigned i = 0; i < cap(buttons); i++)
    {
        Vector2 mousePos = GetMousePosition();

        switch(buttons[i].id)
        {
            case ID_BUTTON_START:

                // Check if button has been pressed
                if(CheckCollisionPointRec(mousePos, buttons[i].rect)) {
                    if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                        StartGame();
                        break;
                    }
                }
            break;
        }
    }

    EndDrawing();
}

void InitializeMainWindow()
{
    int width, height;

    width = GetScreenWidth();
    height = GetScreenHeight();

    // Initialize game window
    InitWindow(GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT, WINDOW_TITLE);

    // Configure FPS
    SetTargetFPS(120);
}
