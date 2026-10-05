#include <raylib.h>
#include <stdbool.h>

#include "game.h"
#include "log.h"
#include "ui.h"

#define MAX_SHELLS 8
#define PLAYER_MAX_HEALTH 3
#define ENEMY_MAX_HEALTH 3
#define PLAYER_SPEED 230.0f
#define ENEMY_SPEED 55.0f
#define SHELL_GRAVITY 640.0f
#define SHELL_SPEED 460.0f

typedef struct Shell {
    Vector2 position;
    Vector2 velocity;
    bool active;
    bool firedByPlayer;
} Shell;

static bool gameRunning = false;
static bool gamePaused = false;
static bool gameOver = false;
static bool resourcesLoaded = false;
static bool audioInitialized = false;

static Texture2D tankTexture;
static Sound shootSound;
static Shell shells[MAX_SHELLS];

static float playerX;
static float enemyX;
static float enemyDirection;
static float playerFireTimer;
static float enemyFireTimer;
static float playerFlashTimer;
static float enemyFlashTimer;
static int playerHealth;
static int enemyHealth;

static void resetRound(void);
static void updateGame(float deltaTime);
static void drawGame(void);
static void fireShell(bool firedByPlayer, float targetX);
static void drawTank(bool faceRight, float x, float y, Color tint);
static void drawHealthBar(int x, int y, const char *label, int health, int maxHealth, Color fill);
static float maxFloat(float a, float b);
static float absFloat(float value);

bool isGameRunning(void)
{
    return gameRunning;
}

void StartGame(void)
{
    if (!resourcesLoaded) {
        InitAudioDevice();
        audioInitialized = IsAudioDeviceReady();

        if (audioInitialized) {
            shootSound = LoadSound("sounds/cannon.wav");
            if (shootSound.frameCount > 0) SetSoundVolume(shootSound, 0.2f);
        } else {
            log_notice("Audio device is unavailable; continuing without sound");
        }

        tankTexture = LoadTexture("textures/sherman2.png");
        if (tankTexture.id == 0) log_error("Could not load textures/sherman2.png");
        resourcesLoaded = true;
    }

    gameRunning = true;
    resetRound();
    log_notice("Starting a new battle");
}

void RunGame(void)
{
    if (IsKeyPressed(KEY_ESCAPE)) {
        gameRunning = false;
        gamePaused = false;
        return;
    }

    if (!gameOver && IsKeyPressed(KEY_P)) gamePaused = !gamePaused;
    if (gameOver && IsKeyPressed(KEY_ENTER)) resetRound();

    if (!gamePaused && !gameOver) updateGame(GetFrameTime());

    BeginDrawing();
    drawGame();
    EndDrawing();
}

void StopGame(void)
{
    gameRunning = false;

    if (resourcesLoaded) {
        if (tankTexture.id != 0) UnloadTexture(tankTexture);
        if (shootSound.frameCount > 0) UnloadSound(shootSound);
        if (audioInitialized) CloseAudioDevice();
    }

    resourcesLoaded = false;
    audioInitialized = false;
}

static void resetRound(void)
{
    playerX = 42.0f;
    enemyX = GAME_WINDOW_WIDTH * 0.78f;
    enemyDirection = -1.0f;
    playerHealth = PLAYER_MAX_HEALTH;
    enemyHealth = ENEMY_MAX_HEALTH;
    playerFireTimer = 0.0f;
    enemyFireTimer = 1.1f;
    playerFlashTimer = 0.0f;
    enemyFlashTimer = 0.0f;
    gamePaused = false;
    gameOver = false;

    for (int i = 0; i < MAX_SHELLS; i++) shells[i].active = false;
}

static float clampFloat(float value, float minValue, float maxValue)
{
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

static float maxFloat(float a, float b)
{
    return a > b ? a : b;
}

static float absFloat(float value)
{
    return value < 0.0f ? -value : value;
}

static void updateGame(float deltaTime)
{
    const float tankWidth = tankTexture.width > 0 ? (float)tankTexture.width : 98.0f;
    const float tankHeight = tankTexture.height > 0 ? (float)tankTexture.height : 44.0f;
    const float enemyMinX = GAME_WINDOW_WIDTH * 0.72f;
    const float enemyMaxX = GAME_WINDOW_WIDTH * 0.88f;
    const float playerMaxX = enemyMinX - tankWidth - 30.0f;
    const float groundY = GAME_WINDOW_HEIGHT * 0.72f;
    float playerMove = 0.0f;

    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) playerMove += 1.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) playerMove -= 1.0f;

    playerX = clampFloat(playerX + playerMove * PLAYER_SPEED * deltaTime, 24.0f, playerMaxX);
    enemyX += enemyDirection * ENEMY_SPEED * deltaTime;
    if (enemyX < enemyMinX || enemyX > enemyMaxX) {
        enemyX = clampFloat(enemyX, enemyMinX, enemyMaxX);
        enemyDirection *= -1.0f;
    }

    playerFireTimer -= deltaTime;
    enemyFireTimer -= deltaTime;
    playerFlashTimer = maxFloat(0.0f, playerFlashTimer - deltaTime);
    enemyFlashTimer = maxFloat(0.0f, enemyFlashTimer - deltaTime);

    if (IsKeyDown(KEY_SPACE) && playerFireTimer <= 0.0f) {
        const float distance = enemyX + tankWidth * 0.5f - (playerX + tankWidth * 0.5f);
        const float leadTime = maxFloat(0.0f, distance / SHELL_SPEED);
        const float targetX = clampFloat(enemyX + tankWidth * 0.5f + enemyDirection * ENEMY_SPEED * leadTime,
                                         enemyMinX + tankWidth * 0.5f,
                                         enemyMaxX + tankWidth * 0.5f);
        fireShell(true, targetX);
        playerFireTimer = 0.55f;
    }

    if (enemyFireTimer <= 0.0f) {
        const float distance = enemyX - playerX;
        const float leadTime = maxFloat(0.0f, distance / SHELL_SPEED);
        const float predictedPlayerX = clampFloat(playerX + playerMove * PLAYER_SPEED * leadTime,
                                                  24.0f, playerMaxX);
        fireShell(false, predictedPlayerX + tankWidth * 0.5f);
        enemyFireTimer = 2.35f;
    }

    const Rectangle playerRect = {playerX, groundY - tankHeight, tankWidth, tankHeight};
    const Rectangle enemyRect = {enemyX, groundY - tankHeight, tankWidth, tankHeight};

    for (int i = 0; i < MAX_SHELLS; i++) {
        Shell *shell = &shells[i];
        if (!shell->active) continue;

        shell->position.x += shell->velocity.x * deltaTime;
        shell->position.y += shell->velocity.y * deltaTime;
        shell->velocity.y += SHELL_GRAVITY * deltaTime;

        const Rectangle shellRect = {shell->position.x - 5.0f, shell->position.y - 4.0f, 10.0f, 8.0f};
        const bool hit = CheckCollisionRecs(shellRect, shell->firedByPlayer ? enemyRect : playerRect);

        if (hit) {
            shell->active = false;
            if (shell->firedByPlayer) {
                enemyHealth--;
                enemyFlashTimer = 0.18f;
                if (enemyHealth <= 0) {
                    enemyHealth = 0;
                    gameOver = true;
                    log_notice("Battle won");
                }
            } else {
                playerHealth--;
                playerFlashTimer = 0.18f;
                if (playerHealth <= 0) {
                    playerHealth = 0;
                    gameOver = true;
                    log_notice("Battle lost");
                }
            }
        } else if (shell->position.y > groundY || shell->position.x < -20.0f || shell->position.x > GAME_WINDOW_WIDTH + 20.0f) {
            shell->active = false;
        }

        if (gameOver) break;
    }

    if (gameOver) {
        for (int i = 0; i < MAX_SHELLS; i++) shells[i].active = false;
    }
}

static void fireShell(bool firedByPlayer, float targetX)
{
    const float tankWidth = tankTexture.width > 0 ? (float)tankTexture.width : 98.0f;
    const float tankHeight = tankTexture.height > 0 ? (float)tankTexture.height : 44.0f;
    const float groundY = GAME_WINDOW_HEIGHT * 0.72f;
    const float startX = firedByPlayer ? playerX + tankWidth * 0.88f : enemyX + tankWidth * 0.12f;
    const float startY = groundY - tankHeight + 17.0f;
    const float deltaX = targetX - startX;
    const float flightTime = clampFloat(absFloat(deltaX) / SHELL_SPEED, 0.35f, 1.5f);
    const float targetY = groundY - tankHeight * 0.55f;

    for (int i = 0; i < MAX_SHELLS; i++) {
        if (!shells[i].active) {
            shells[i].position = (Vector2){startX, startY};
            shells[i].velocity.x = deltaX / flightTime;
            shells[i].velocity.y = (targetY - startY - 0.5f * SHELL_GRAVITY * flightTime * flightTime) / flightTime;
            shells[i].active = true;
            shells[i].firedByPlayer = firedByPlayer;
            if (shootSound.frameCount > 0) PlaySound(shootSound);
            return;
        }
    }
}

static void drawGame(void)
{
    const float tankWidth = tankTexture.width > 0 ? (float)tankTexture.width : 98.0f;
    const float tankHeight = tankTexture.height > 0 ? (float)tankTexture.height : 44.0f;
    const float groundY = GAME_WINDOW_HEIGHT * 0.72f;
    const Color playerTint = playerFlashTimer > 0.0f ? ORANGE : WHITE;
    const Color enemyTint = enemyFlashTimer > 0.0f ? ORANGE : (Color){220, 112, 92, 255};

    DrawRectangleGradientV(0, 0, GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT, (Color){99, 164, 206, 255}, (Color){213, 226, 214, 255});
    DrawCircle(125, 78, 28.0f, (Color){255, 231, 159, 255});
    DrawCircle(312, 83, 21.0f, Fade(RAYWHITE, 0.72f));
    DrawCircle(337, 78, 28.0f, Fade(RAYWHITE, 0.72f));
    DrawCircle(365, 84, 19.0f, Fade(RAYWHITE, 0.72f));
    DrawCircle(770, 103, 19.0f, Fade(RAYWHITE, 0.62f));
    DrawCircle(796, 96, 28.0f, Fade(RAYWHITE, 0.62f));
    DrawCircle(825, 104, 18.0f, Fade(RAYWHITE, 0.62f));

    DrawTriangle((Vector2){0, groundY}, (Vector2){225, 208}, (Vector2){470, groundY}, (Color){106, 139, 117, 255});
    DrawTriangle((Vector2){430, groundY}, (Vector2){722, 190}, (Vector2){1024, groundY}, (Color){119, 151, 122, 255});
    DrawRectangle(0, (int)groundY, GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT - (int)groundY, (Color){115, 99, 75, 255});
    DrawRectangle(0, (int)groundY, GAME_WINDOW_WIDTH, 9, (Color){92, 118, 76, 255});
    DrawLine(0, (int)groundY + 44, GAME_WINDOW_WIDTH, (int)groundY + 44, Fade(BLACK, 0.12f));

    DrawEllipse((int)(playerX + tankWidth * 0.5f), (int)(groundY + 1.0f), (int)(tankWidth * 0.46f), 7, Fade(BLACK, 0.22f));
    DrawEllipse((int)(enemyX + tankWidth * 0.5f), (int)(groundY + 1.0f), (int)(tankWidth * 0.46f), 7, Fade(BLACK, 0.22f));
    drawTank(true, playerX, groundY - tankHeight, playerTint);
    drawTank(false, enemyX, groundY - tankHeight, enemyTint);

    for (int i = 0; i < MAX_SHELLS; i++) {
        if (shells[i].active) {
            DrawCircleV(shells[i].position, 5.0f, (Color){255, 203, 105, 255});
            DrawCircleV(shells[i].position, 2.0f, RAYWHITE);
        }
    }

    drawHealthBar(28, 24, "PLAYER", playerHealth, PLAYER_MAX_HEALTH, (Color){78, 194, 118, 255});
    drawHealthBar(GAME_WINDOW_WIDTH - 190, 24, "ENEMY", enemyHealth, ENEMY_MAX_HEALTH, (Color){221, 90, 76, 255});
    DrawText("A/D or arrows: move     SPACE: fire     P: pause     ESC: menu", 184, GAME_WINDOW_HEIGHT - 28, 15, RAYWHITE);

    if (gamePaused) {
        DrawRectangle(0, 0, GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT, Fade(BLACK, 0.48f));
        const char *message = "PAUSED";
        DrawText(message, (GAME_WINDOW_WIDTH - MeasureText(message, 48)) / 2, 196, 48, RAYWHITE);
        DrawText("Press P to continue", (GAME_WINDOW_WIDTH - MeasureText("Press P to continue", 20)) / 2, 256, 20, RAYWHITE);
    } else if (gameOver) {
        DrawRectangle(0, 0, GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT, Fade(BLACK, 0.56f));
        const char *message = enemyHealth == 0 ? "VICTORY!" : "DEFEAT";
        DrawText(message, (GAME_WINDOW_WIDTH - MeasureText(message, 52)) / 2, 180, 52,
                 enemyHealth == 0 ? GOLD : (Color){255, 135, 116, 255});
        DrawText("Press ENTER to play again or ESC for menu", (GAME_WINDOW_WIDTH - MeasureText("Press ENTER to play again or ESC for menu", 20)) / 2,
                 250, 20, RAYWHITE);
    }
}

static void drawTank(bool faceRight, float x, float y, Color tint)
{
    if (tankTexture.id == 0) {
        DrawRectangle((int)x, (int)y + 15, 98, 29, tint);
        DrawRectangle((int)x + (faceRight ? 50 : 12), (int)y + 8, 38, 10, tint);
        return;
    }

    const float width = (float)tankTexture.width;
    const float height = (float)tankTexture.height;
    const Rectangle source = faceRight
        ? (Rectangle){0.0f, 0.0f, width, height}
        : (Rectangle){width, 0.0f, -width, height};
    const Rectangle destination = {x, y, width, height};
    DrawTexturePro(tankTexture, source, destination, (Vector2){0.0f, 0.0f}, 0.0f, tint);
}

static void drawHealthBar(int x, int y, const char *label, int health, int maxHealth, Color fill)
{
    DrawText(label, x, y, 16, RAYWHITE);
    for (int i = 0; i < maxHealth; i++) {
        const int segmentX = x + i * 48;
        DrawRectangle(segmentX, y + 23, 40, 12, Fade(BLACK, 0.34f));
        if (i < health) DrawRectangle(segmentX + 2, y + 25, 36, 8, fill);
    }
}
