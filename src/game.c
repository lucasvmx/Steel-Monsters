#include <raylib.h>
#include <math.h>
#include <stdbool.h>

#include "game.h"
#include "log.h"
#include "ui.h"

#define PLAYER_MAX_HEALTH 3
#define ENEMY_MAX_HEALTH 3
#define TANK_WIDTH 98.0f
#define TANK_HEIGHT 44.0f
#define TERRAIN_BASE_Y (GAME_WINDOW_HEIGHT * 0.72f)
#define SHELL_GRAVITY 520.0f
#define WIND_ACCELERATION 11.0f
#define PLAYER_MOVE_SPEED 190.0f
#define PLAYER_MOVE_PER_TURN 115.0f
#define BLAST_RADIUS 43.0f
#define EXPLOSION_DURATION 0.32f

#define DEG_TO_RAD 0.01745329251994329577f

typedef enum BattleTurn {
    TURN_PLAYER,
    TURN_ENEMY
} BattleTurn;

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
static Shell shell;

static float terrain[GAME_WINDOW_WIDTH + 1];
static float playerX;
static float enemyX;
static float playerAngle;
static float enemyAngle;
static float playerPower;
static float enemyPower;
static float playerMoveRemaining;
static float enemyAimTimer;
static float turnDelay;
static float explosionTimer;
static float playerFlashTimer;
static float enemyFlashTimer;
static Vector2 explosionPosition;
static float wind;
static int playerHealth;
static int enemyHealth;
static BattleTurn currentTurn;

static void resetRound(void);
static void updateGame(float deltaTime);
static void drawGame(void);
static void fireShell(bool firedByPlayer, float angle, float power);
static void drawTank(bool faceRight, float x, float groundY, float angle, Color tint);
static void drawHealthBar(int x, int y, const char *label, int health, int maxHealth, Color fill);
static void chooseEnemyShot(float *angle, float *power);
static void explodeShell(bool directHit);
static void applyCrater(float x, float radius, float depth);
static float groundAt(float x);
static float clampFloat(float value, float minValue, float maxValue);
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
    log_notice("Starting a new artillery battle");
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

static float groundAt(float x)
{
    const int index = (int)clampFloat(x, 0.0f, (float)GAME_WINDOW_WIDTH);
    return terrain[index];
}

static void resetRound(void)
{
    for (int x = 0; x <= GAME_WINDOW_WIDTH; x++) {
        const float xf = (float)x;
        terrain[x] = TERRAIN_BASE_Y
            + 27.0f * sinf(xf * 0.0090f)
            + 13.0f * sinf(xf * 0.023f + 1.1f)
            + 6.0f * sinf(xf * 0.047f + 2.4f);
    }

    playerX = 40.0f;
    enemyX = GAME_WINDOW_WIDTH - TANK_WIDTH - 46.0f;
    playerAngle = 42.0f;
    enemyAngle = 42.0f;
    playerPower = 63.0f;
    enemyPower = 63.0f;
    playerMoveRemaining = PLAYER_MOVE_PER_TURN;
    playerHealth = PLAYER_MAX_HEALTH;
    enemyHealth = ENEMY_MAX_HEALTH;
    currentTurn = TURN_PLAYER;
    wind = (float)GetRandomValue(-5, 5);
    enemyAimTimer = 0.0f;
    turnDelay = 0.0f;
    explosionTimer = 0.0f;
    playerFlashTimer = 0.0f;
    enemyFlashTimer = 0.0f;
    shell.active = false;
    gamePaused = false;
    gameOver = false;
}

static void updateGame(float deltaTime)
{
    const bool playerCanAct = currentTurn == TURN_PLAYER && !shell.active && turnDelay <= 0.0f;

    playerFlashTimer = maxFloat(0.0f, playerFlashTimer - deltaTime);
    enemyFlashTimer = maxFloat(0.0f, enemyFlashTimer - deltaTime);
    explosionTimer = maxFloat(0.0f, explosionTimer - deltaTime);

    if (turnDelay > 0.0f) {
        turnDelay -= deltaTime;
        if (turnDelay <= 0.0f && !gameOver) {
            currentTurn = currentTurn == TURN_PLAYER ? TURN_ENEMY : TURN_PLAYER;
            wind = (float)GetRandomValue(-5, 5);
            if (currentTurn == TURN_PLAYER) {
                playerMoveRemaining = PLAYER_MOVE_PER_TURN;
            } else {
                enemyAimTimer = 0.85f;
            }
        }
    }

    if (playerCanAct) {
        if (IsKeyDown(KEY_UP)) playerAngle += 42.0f * deltaTime;
        if (IsKeyDown(KEY_DOWN)) playerAngle -= 42.0f * deltaTime;
        playerAngle = clampFloat(playerAngle, 10.0f, 82.0f);

        if (IsKeyDown(KEY_W)) playerPower += 48.0f * deltaTime;
        if (IsKeyDown(KEY_S)) playerPower -= 48.0f * deltaTime;
        playerPower = clampFloat(playerPower, 25.0f, 100.0f);

        float movement = 0.0f;
        if (IsKeyDown(KEY_D)) movement += 1.0f;
        if (IsKeyDown(KEY_A)) movement -= 1.0f;
        if (movement != 0.0f && playerMoveRemaining > 0.0f) {
            float distance = movement * PLAYER_MOVE_SPEED * deltaTime;
            if (absFloat(distance) > playerMoveRemaining) {
                distance = movement * playerMoveRemaining;
            }
            const float maxX = maxFloat(20.0f, enemyX - TANK_WIDTH - 28.0f);
            const float oldX = playerX;
            playerX = clampFloat(playerX + distance, 20.0f, maxX);
            playerMoveRemaining = maxFloat(0.0f, playerMoveRemaining - absFloat(playerX - oldX));
        }

        if (IsKeyPressed(KEY_SPACE)) {
            fireShell(true, playerAngle, playerPower);
        }
    } else if (currentTurn == TURN_ENEMY && !shell.active && turnDelay <= 0.0f) {
        enemyAimTimer -= deltaTime;
        if (enemyAimTimer <= 0.0f) {
            chooseEnemyShot(&enemyAngle, &enemyPower);
            fireShell(false, enemyAngle, enemyPower);
        }
    }

    if (shell.active) {
        const float targetX = shell.firedByPlayer ? enemyX : playerX;
        const float targetGroundY = groundAt(targetX + TANK_WIDTH * 0.5f);
        const Rectangle targetRect = {
            targetX + 7.0f,
            targetGroundY - TANK_HEIGHT + 9.0f,
            TANK_WIDTH - 14.0f,
            TANK_HEIGHT - 12.0f
        };

        shell.position.x += shell.velocity.x * deltaTime;
        shell.position.y += shell.velocity.y * deltaTime;
        shell.velocity.x += wind * WIND_ACCELERATION * deltaTime;
        shell.velocity.y += SHELL_GRAVITY * deltaTime;

        const Rectangle shellRect = {shell.position.x - 5.0f, shell.position.y - 5.0f, 10.0f, 10.0f};
        if (CheckCollisionRecs(shellRect, targetRect)) {
            explodeShell(true);
        } else if (shell.position.x < 0.0f || shell.position.x > GAME_WINDOW_WIDTH
                   || shell.position.y > groundAt(shell.position.x)) {
            explodeShell(false);
        } else if (shell.position.y < -40.0f) {
            shell.active = false;
            turnDelay = 0.45f;
        }
    }
}

static void fireShell(bool firedByPlayer, float angle, float power)
{
    const float direction = firedByPlayer ? 1.0f : -1.0f;
    const float radians = angle * DEG_TO_RAD;
    const float tankX = firedByPlayer ? playerX : enemyX;
    const float tankCenterX = tankX + TANK_WIDTH * 0.5f;
    const float startX = tankCenterX + direction * 21.0f;
    const float startY = groundAt(tankCenterX) - 28.0f;
    const float speed = 275.0f + power * 6.15f;

    shell.position = (Vector2){startX, startY};
    shell.velocity = (Vector2){direction * cosf(radians) * speed, -sinf(radians) * speed};
    shell.active = true;
    shell.firedByPlayer = firedByPlayer;
    if (shootSound.frameCount > 0) PlaySound(shootSound);
}

static void chooseEnemyShot(float *angle, float *power)
{
    const float direction = -1.0f;
    const float enemyCenterX = enemyX + TANK_WIDTH * 0.5f;
    const float startX = enemyCenterX - 21.0f;
    const float startY = groundAt(enemyCenterX) - 28.0f;
    const float targetLeft = playerX + 7.0f;
    const float targetRight = playerX + TANK_WIDTH - 7.0f;
    const float targetTop = groundAt(playerX + TANK_WIDTH * 0.5f) - TANK_HEIGHT + 9.0f;
    const float targetBottom = targetTop + TANK_HEIGHT - 12.0f;
    float bestScore = 1000000.0f;
    float bestAngle = 45.0f;
    float bestPower = 65.0f;

    for (int angleStep = 15; angleStep <= 82; angleStep += 1) {
        const float radians = (float)angleStep * DEG_TO_RAD;
        for (int powerStep = 25; powerStep <= 100; powerStep += 1) {
            const float speed = 275.0f + (float)powerStep * 6.15f;
            float x = startX;
            float y = startY;
            float vx = direction * cosf(radians) * speed;
            float vy = -sinf(radians) * speed;
            float score = 1000000.0f;

            for (int step = 0; step < 150; step++) {
                const float dt = 0.025f;
                x += vx * dt;
                y += vy * dt;
                vx += wind * WIND_ACCELERATION * dt;
                vy += SHELL_GRAVITY * dt;

                if (x < 0.0f || x > GAME_WINDOW_WIDTH) break;

                const float nearestX = clampFloat(x, targetLeft, targetRight);
                const float nearestY = clampFloat(y, targetTop, targetBottom);
                const float dx = absFloat(x - nearestX);
                const float dy = absFloat(y - nearestY);
                const float distance = dx + dy * 1.4f;
                if (distance < score) score = distance;
                if (distance < 1.0f) break;
                if (y >= groundAt(x)) break;
            }

            if (score < bestScore) {
                bestScore = score;
                bestAngle = (float)angleStep;
                bestPower = (float)powerStep;
            }
        }
    }

    *angle = bestAngle;
    *power = bestPower;
}

static void explodeShell(bool directHit)
{
    const bool playerShot = shell.firedByPlayer;
    const float blastX = shell.position.x;
    const float otherTankX = playerShot ? enemyX : playerX;
    const float otherTankCenter = otherTankX + TANK_WIDTH * 0.5f;
    const float horizontalDistance = absFloat(blastX - otherTankCenter);
    const bool blastHit = horizontalDistance < BLAST_RADIUS + 14.0f;

    explosionPosition = shell.position;
    explosionTimer = EXPLOSION_DURATION;
    shell.active = false;
    applyCrater(blastX, BLAST_RADIUS, 29.0f);

    if (directHit || blastHit) {
        if (playerShot) {
            enemyHealth--;
            enemyFlashTimer = 0.25f;
            if (enemyHealth <= 0) {
                enemyHealth = 0;
                gameOver = true;
                log_notice("Battle won");
            }
        } else {
            playerHealth--;
            playerFlashTimer = 0.25f;
            if (playerHealth <= 0) {
                playerHealth = 0;
                gameOver = true;
                log_notice("Battle lost");
            }
        }
    }

    if (gameOver) {
        turnDelay = 0.0f;
        return;
    }

    turnDelay = 0.72f;
}

static void applyCrater(float x, float radius, float depth)
{
    const int center = (int)clampFloat(x, 0.0f, (float)GAME_WINDOW_WIDTH);
    const int extent = (int)radius;
    const int start = center - extent < 0 ? 0 : center - extent;
    const int end = center + extent > GAME_WINDOW_WIDTH ? GAME_WINDOW_WIDTH : center + extent;

    for (int px = start; px <= end; px++) {
        const float normalized = absFloat((float)px - (float)center) / radius;
        const float craterDepth = depth * (1.0f - normalized * normalized);
        terrain[px] += craterDepth;
    }
}

static void drawGame(void)
{
    const float playerGroundY = groundAt(playerX + TANK_WIDTH * 0.5f);
    const float enemyGroundY = groundAt(enemyX + TANK_WIDTH * 0.5f);
    const Color playerTint = playerFlashTimer > 0.0f ? ORANGE : WHITE;
    const Color enemyTint = enemyFlashTimer > 0.0f ? ORANGE : (Color){220, 112, 92, 255};

    DrawRectangleGradientV(0, 0, GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT,
                           (Color){99, 164, 206, 255}, (Color){213, 226, 214, 255});
    DrawCircle(125, 78, 28.0f, (Color){255, 231, 159, 255});
    DrawCircle(312, 83, 21.0f, Fade(RAYWHITE, 0.72f));
    DrawCircle(337, 78, 28.0f, Fade(RAYWHITE, 0.72f));
    DrawCircle(365, 84, 19.0f, Fade(RAYWHITE, 0.72f));
    DrawCircle(770, 103, 19.0f, Fade(RAYWHITE, 0.62f));
    DrawCircle(796, 96, 28.0f, Fade(RAYWHITE, 0.62f));
    DrawCircle(825, 104, 18.0f, Fade(RAYWHITE, 0.62f));

    DrawTriangle((Vector2){0.0f, TERRAIN_BASE_Y}, (Vector2){225.0f, 208.0f},
                 (Vector2){470.0f, TERRAIN_BASE_Y}, (Color){106, 139, 117, 255});
    DrawTriangle((Vector2){430.0f, TERRAIN_BASE_Y}, (Vector2){722.0f, 190.0f},
                 (Vector2){1024.0f, TERRAIN_BASE_Y}, (Color){119, 151, 122, 255});

    for (int x = 0; x < GAME_WINDOW_WIDTH; x++) {
        const Vector2 topLeft = {(float)x, terrain[x]};
        const Vector2 topRight = {(float)x + 1.0f, terrain[x + 1]};
        const Vector2 bottomLeft = {(float)x, (float)GAME_WINDOW_HEIGHT};
        const Vector2 bottomRight = {(float)x + 1.0f, (float)GAME_WINDOW_HEIGHT};
        DrawTriangle(topLeft, topRight, bottomRight, (Color){115, 99, 75, 255});
        DrawTriangle(topLeft, bottomRight, bottomLeft, (Color){115, 99, 75, 255});
        DrawLineV(topLeft, topRight, (Color){92, 118, 76, 255});
    }

    DrawEllipse((int)(playerX + TANK_WIDTH * 0.5f), (int)(playerGroundY + 1.0f), 44, 7, Fade(BLACK, 0.22f));
    DrawEllipse((int)(enemyX + TANK_WIDTH * 0.5f), (int)(enemyGroundY + 1.0f), 44, 7, Fade(BLACK, 0.22f));
    drawTank(true, playerX, playerGroundY, playerAngle, playerTint);
    drawTank(false, enemyX, enemyGroundY, enemyAngle, enemyTint);

    if (shell.active) {
        DrawCircleV(shell.position, 5.0f, (Color){255, 203, 105, 255});
        DrawCircleV(shell.position, 2.0f, RAYWHITE);
    }

    if (explosionTimer > 0.0f) {
        const float progress = 1.0f - explosionTimer / EXPLOSION_DURATION;
        const float radius = 10.0f + progress * 35.0f;
        DrawCircleV(explosionPosition, radius, Fade((Color){255, 135, 68, 255}, 1.0f - progress));
        DrawCircleV(explosionPosition, radius * 0.55f, Fade((Color){255, 219, 122, 255}, 1.0f - progress));
    }

    DrawRectangle(0, 0, GAME_WINDOW_WIDTH, 78, Fade((Color){20, 35, 39, 255}, 0.88f));
    drawHealthBar(24, 12, "PLAYER", playerHealth, PLAYER_MAX_HEALTH, (Color){78, 194, 118, 255});
    drawHealthBar(GAME_WINDOW_WIDTH - 168, 12, "ENEMY", enemyHealth, ENEMY_MAX_HEALTH, (Color){221, 90, 76, 255});
    const char *turnLabel = shell.active
        ? (shell.firedByPlayer ? "PLAYER'S SHOT" : "ENEMY'S SHOT")
        : (turnDelay > 0.0f ? "BLAST!" : (currentTurn == TURN_PLAYER ? "YOUR TURN" : "ENEMY AIMING"));
    DrawText(turnLabel, (GAME_WINDOW_WIDTH - MeasureText(turnLabel, 22)) / 2, 11, 22, RAYWHITE);
    DrawText(TextFormat("WIND %s %d", wind < 0.0f ? "<" : ">", (int)absFloat(wind)), 461, 43, 17, (Color){199, 218, 219, 255});
    DrawLine(500, 64, wind < 0.0f ? 473 : 527, 64, (Color){199, 218, 219, 255});
    DrawTriangle((Vector2){wind < 0.0f ? 473.0f : 527.0f, 64.0f},
                 (Vector2){wind < 0.0f ? 481.0f : 519.0f, 59.0f},
                 (Vector2){wind < 0.0f ? 481.0f : 519.0f, 69.0f}, (Color){199, 218, 219, 255});

    DrawRectangle(0, GAME_WINDOW_HEIGHT - 65, GAME_WINDOW_WIDTH, 65, Fade((Color){20, 35, 39, 255}, 0.88f));
    DrawText(TextFormat("ANGLE %d deg", (int)(playerAngle + 0.5f)), 26, GAME_WINDOW_HEIGHT - 51, 17, RAYWHITE);
    DrawText(TextFormat("POWER %d%%", (int)(playerPower + 0.5f)), 208, GAME_WINDOW_HEIGHT - 51, 17, RAYWHITE);
    DrawText(TextFormat("MOVE %d", (int)(playerMoveRemaining + 0.5f)), 380, GAME_WINDOW_HEIGHT - 51, 17, RAYWHITE);
    DrawText("UP/DOWN AIM   W/S POWER   A/D MOVE   SPACE FIRE   P PAUSE   ESC MENU",
             510, GAME_WINDOW_HEIGHT - 51, 15, RAYWHITE);

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

static void drawTank(bool faceRight, float x, float groundY, float angle, Color tint)
{
    const Rectangle destination = {x, groundY - TANK_HEIGHT, TANK_WIDTH, TANK_HEIGHT};
    const Rectangle source = faceRight
        ? (Rectangle){0.0f, 0.0f, TANK_WIDTH, TANK_HEIGHT}
        : (Rectangle){TANK_WIDTH, 0.0f, -TANK_WIDTH, TANK_HEIGHT};
    const float direction = faceRight ? 1.0f : -1.0f;
    const float pivotX = x + TANK_WIDTH * 0.5f;
    const float pivotY = groundY - TANK_HEIGHT + 19.0f;
    const float radians = angle * DEG_TO_RAD;
    const Vector2 muzzle = {
        pivotX + direction * cosf(radians) * 43.0f,
        pivotY - sinf(radians) * 43.0f
    };
    const Color barrelShadow = faceRight ? (Color){26, 58, 18, 255} : (Color){115, 54, 44, 255};
    const Color barrelFill = faceRight ? (Color){54, 91, 39, 255} : (Color){178, 85, 67, 255};

    if (tankTexture.id != 0) {
        DrawTexturePro(tankTexture, source, destination, (Vector2){0.0f, 0.0f}, 0.0f, tint);
    } else {
        DrawRectangleRounded((Rectangle){x + 7.0f, groundY - 27.0f, TANK_WIDTH - 14.0f, 20.0f}, 0.28f, 5, tint);
        DrawRectangleRounded((Rectangle){x + 19.0f, groundY - 39.0f, 55.0f, 16.0f}, 0.28f, 5, tint);
        DrawRectangleRounded((Rectangle){x + 6.0f, groundY - 13.0f, TANK_WIDTH - 12.0f, 12.0f}, 0.35f, 5, DARKGRAY);
        for (int wheel = 0; wheel < 4; wheel++) DrawCircle((int)(x + 20.0f + wheel * 19.0f), (int)(groundY - 7.0f), 5.0f, GRAY);
    }

    DrawLineEx((Vector2){pivotX, pivotY}, muzzle, barrelShadow, 8.0f);
    DrawLineEx((Vector2){pivotX, pivotY}, muzzle, barrelFill, 4.0f);
    DrawCircleV((Vector2){pivotX, pivotY}, 7.0f, barrelShadow);
    DrawCircleV((Vector2){pivotX, pivotY}, 4.5f, barrelFill);
}

static void drawHealthBar(int x, int y, const char *label, int health, int maxHealth, Color fill)
{
    DrawText(label, x, y, 16, RAYWHITE);
    for (int i = 0; i < maxHealth; i++) {
        const int segmentX = x + i * 43;
        DrawRectangle(segmentX, y + 22, 35, 11, Fade(BLACK, 0.34f));
        if (i < health) DrawRectangle(segmentX + 2, y + 24, 31, 7, fill);
    }
}
