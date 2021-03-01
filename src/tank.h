
#ifndef TANK_H
#define TANK_H

#include <raylib.h>

extern void DrawTank(Texture2D tank, int x, int y, Color color);
extern Texture2D LoadTank();
extern void DrawTankEx(Texture2D tank, int x, int y, Color color);
extern Image LoadTankImage();

#endif
