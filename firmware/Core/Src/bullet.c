/*
 * Check artifact flicker on display bc. Upgrades might not be possible due to too much artifacting
 */

#include "bullet.h"
#include "spaceship.h"
#include <math.h>

void Bullets_Init(BulletPool *pool)
{
    for (int i = 0; i < MAX_BULLETS; i++) {
        pool->bullets[i].active = 0;
    }
    pool->count = 0;
}

void Bullets_Spawn(BulletPool *pool, float x, float y, float angle)
{

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!pool->bullets[i].active) {
            Bullet *bullet = &pool->bullets[i];
            bullet->active = 1;

            //Check for free bullet spots
            bullet->x  = x + sinf(angle) * SHIP_SIZE;
            bullet->y  = y - cosf(angle) * SHIP_SIZE;

            bullet->prevX = bullet->x;
            bullet->prevY = bullet->y;

            //Move Bullet by Bullet_speed var
            bullet->dx = sinf(angle) * BULLET_SPEED;
            bullet->dy = -cosf(angle) * BULLET_SPEED;
            return;
        }
    }
    // if theres no free slot oldest bullet gets overwritten --> Check for RAM Usage
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (pool->bullets[i].active) {

            Bullet *bullet = &pool->bullets[i];

            bullet->x  = x + sinf(angle) * SHIP_SIZE;
            bullet->y  = y - cosf(angle) * SHIP_SIZE;

            bullet->prevX = bullet->x;
            bullet->prevY = bullet->y;

            bullet->dx = sinf(angle) * BULLET_SPEED;
            bullet->dy = -cosf(angle) * BULLET_SPEED;
            return;
        }
    }
}

void Bullets_Update(BulletPool *pool)
{
    for (int i = 0; i < MAX_BULLETS; i++) {

        Bullet *bullet = &pool->bullets[i];

        if (!bullet->active) continue;

        bullet->prevX = bullet->x;
        bullet->prevY = bullet->y;

        bullet->x += bullet->dx;
        bullet->y += bullet->dy;


        if (bullet->x < 0 || bullet->x > SCREEN_W || bullet->y < 0 || bullet->y > SCREEN_H) {
            bullet->active = 0;
        }
    }
}

void Bullets_Draw(BulletPool *pool)
{
    for (int i = 0; i < MAX_BULLETS; i++) {
        Bullet *bullet = &pool->bullets[i];

        // Erase previous position
        if (bullet->prevX != bullet->x || bullet->prevY != bullet->y) {
            ILI9341_FillRectangle(
                (uint16_t)(bullet->prevX - BULLET_RADIUS),
                (uint16_t)(bullet->prevY - BULLET_RADIUS),
                BULLET_RADIUS * 2, BULLET_RADIUS * 2,
                COLOR_BLACK);
        }

        // Draw at new position
        if (bullet->active) {
            ILI9341_FillRectangle(
                (uint16_t)(bullet->x - BULLET_RADIUS),
                (uint16_t)(bullet->y - BULLET_RADIUS),
                BULLET_RADIUS * 2, BULLET_RADIUS * 2,
                ILI9341_YELLOW);
        }
    }
}
