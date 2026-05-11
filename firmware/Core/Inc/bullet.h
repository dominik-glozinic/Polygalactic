/*
 * bullet.h
 *
 *  Created on: Apr 19, 2026
 *      Author: domy
 */

#ifndef INC_BULLET_H_
#define INC_BULLET_H_

#include "ili9341.h"
#include <stdint.h>

#define MAX_BULLETS     16
#define BULLET_SPEED    5.0f
#define BULLET_RADIUS   2

typedef struct {
    float    x, y;
    float    dx, dy;      // velocity components
    uint8_t  active;
    float    prevX, prevY;
} Bullet;

typedef struct {
    Bullet   bullets[MAX_BULLETS];
    uint8_t  count;
} BulletPool;

void Bullets_Init(BulletPool *pool);
void Bullets_Spawn(BulletPool *pool, float x, float y, float angle);
void Bullets_Update(BulletPool *pool);
void Bullets_Draw(BulletPool *pool);

#endif /* INC_BULLET_H_ */
