#include "tank.h"

Image LoadTankImage()
{
    return LoadImage("textures/sherman2.png");
}

Texture2D LoadTank()
{
    return LoadTexture("textures/sherman2.png");
}

void DrawTank(Texture2D tank, int x, int y, Color color)
{
    DrawTexture(tank, x, y, color);
}

void DrawTankEx(Texture2D tank, int x, int y, Color color)
{
    Rectangle r = {
        .width = tank.width / 2,
        .height = tank.height / 7,
        .x = 0,
        .y = 0
    };

    Vector2 v = {50, 50};

    DrawTextureRec(tank, r, v, color);
}


