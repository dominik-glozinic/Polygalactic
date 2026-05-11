/*
 * spaceship.c
 */

#include "spaceship.h"
#include <math.h>
#include <stdlib.h>


static void drawShipTriangle(float currX, float currY, float angle, uint16_t color)
{
    float sinA = sinf(angle);
    float cosA = cosf(angle);

    //The vertices (corners) before rotation
    float verX[3] = {  0.0f,          -SHIP_SIZE * 0.6f,  SHIP_SIZE * 0.6f };
    float verY[3] = { -SHIP_SIZE,      SHIP_SIZE * 0.8f,  SHIP_SIZE * 0.8f };

    //The final position on screen
    int16_t scrX[3], scrY[3];

    for (int i = 0; i < 3; i++) {
        scrX[i] = (int16_t)(currX + verX[i] * cosA - verY[i] * sinA);
        scrY[i] = (int16_t)(currY + verX[i] * sinA + verY[i] * cosA);
    }

    ILI9341_DrawLine(scrX[0], scrY[0], scrX[1], scrY[1], color);
    ILI9341_DrawLine(scrX[1], scrY[1], scrX[2], scrY[2], color);
    ILI9341_DrawLine(scrX[2], scrY[2], scrX[0], scrY[0], color);
}


void Ship_Init(Spaceship *ship)
{
    ship->x            = SCREEN_W / 2.0f;
    ship->y            = SCREEN_H / 2.0f;
    ship->angle        = 0.0f;
    ship->prevX        = ship->x;
    ship->prevY        = ship->y;
    ship->prevAngle    = ship->angle;
    ship->needsErase   = 0;
    ship->lastShotTick = 0;

    ship->hp           = SHIP_MAX_HP;
    ship->invulnFrames = 0;
}


void Ship_Update(Spaceship *ship, uint32_t *adcBuffer)
{
    ship->prevX      = ship->x;
    ship->prevY      = ship->y;
    ship->prevAngle  = ship->angle;
    ship->needsErase = 1;

    //Left joystick = position
    /*
     * NOTE: The joysticks might need to be rotated after the upgrade
     */
    int32_t ly = ((int32_t)adcBuffer[0] - JOY_CENTRE);
    int32_t lx =  -((int32_t)adcBuffer[1] - JOY_CENTRE);

    if (abs(lx) < JOY_DEADZONE) lx = 0;
    if (abs(ly) < JOY_DEADZONE) ly = 0;

    float lx_norm = lx >= 0 ? (float)lx / JOY_MAX_POS : (float)lx / JOY_MAX_NEG;
    float ly_norm = ly >= 0 ? (float)ly / JOY_MAX_POS : (float)ly / JOY_MAX_NEG;

    if (lx_norm >  1.0f) lx_norm =  1.0f;
    if (lx_norm < -1.0f) lx_norm = -1.0f;
    if (ly_norm >  1.0f) ly_norm =  1.0f;
    if (ly_norm < -1.0f) ly_norm = -1.0f;

    ship->x += lx_norm * SHIP_SPEED;
    ship->y += ly_norm * SHIP_SPEED;


    if (ship->x < SHIP_SIZE)             ship->x = SHIP_SIZE;
    if (ship->x > SCREEN_W - SHIP_SIZE)  ship->x = SCREEN_W - SHIP_SIZE;
    if (ship->y < SHIP_SIZE)             ship->y = SHIP_SIZE;
    if (ship->y > SCREEN_H - SHIP_SIZE)  ship->y = SCREEN_H - SHIP_SIZE;

    // Right joystick = rotation
    int32_t ry = -((int32_t)adcBuffer[2] - JOY_CENTRE);
    int32_t rx =   -((int32_t)adcBuffer[3] - JOY_CENTRE);

    if (abs(rx) > JOY_DEADZONE || abs(ry) > JOY_DEADZONE) {
        ship->angle = atan2f((float)rx, -(float)ry) - (3.14159265f / 2.0f);
    }


    if (ship->invulnFrames > 0) {
        ship->invulnFrames--;
    }
}


void Ship_Draw(Spaceship *ship)
{

    if (ship->needsErase) {
        drawShipTriangle(ship->prevX, ship->prevY,
                         ship->prevAngle, COLOR_BLACK);
    }


    if (ship->invulnFrames > 0) {
        if ((ship->invulnFrames / SHIP_FLICKER_RATE) % 2 == 0) {
            return;
        }
    }

    drawShipTriangle(ship->x, ship->y, ship->angle, COLOR_CYAN);
}


uint8_t Ship_TakeDamage(Spaceship *ship)
{
    if (ship->invulnFrames > 0) return 0;

    ship->hp--;
    ship->invulnFrames = SHIP_INVULN_FRAMES;

    if (ship->hp <= 0) {
        ship->hp = 0;
        return 1;   // 1 if ship is destroyed
    }
    return 0;
}
