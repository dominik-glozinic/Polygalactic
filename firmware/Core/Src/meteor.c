/*
 * meteor.c
 */

#include "meteor.h"
#include "bullet.h"
#include <math.h>
#include <stdlib.h>

#define METEOR_COLOR  ILI9341_ORANGE

// Fill rectangle so it lessens the flicker --> DOESNT WORK!!!!!!!!!!!!!!!!
static void _fill(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t c)
{
    if (width <= 0 || height <= 0)               return;
    if (x >= SCREEN_W || y >= SCREEN_H) return;
    if (x + width <= 0    || y + height <= 0)    return;
    if (x < 0) { width += x; x = 0; }
    if (y < 0) { height += y; y = 0; }
    if (x + width > SCREEN_W) width = SCREEN_W - x;
    if (y + height > SCREEN_H) height = SCREEN_H - y;
    if (width <= 0 || height <= 0) return;
    ILI9341_FillRectangle((uint16_t)x, (uint16_t)y,
                           (uint16_t)width, (uint16_t)height, c);
}



static void _draw_octagon(int16_t cenX, int16_t cenY, uint8_t r, uint16_t color)
{
    int16_t intR  = (int16_t)r;
    // cut = r * 0.414, integer --> gives equal-length sides
    int16_t cut = (int16_t)((uint32_t)r * 414 / 1000);
    if (cut < 1)       cut = 1;
    if (cut > intR - 1)  cut = intR - 1;


    _fill(cenX - intR,
          cenY - (intR - cut),
          2 * intR,
          2 * (intR - cut) + 1,
          color);

    // 2. Chamfer rows — top and bottom, stepping inward
    for (int16_t k = 1; k <= cut; k++) {
        int16_t rw = 2 * (intR - k) + 1;
        int16_t rx = cenX - (intR - k);
        /* top */
        _fill(rx, cenY - (intR - cut) - k, rw, 1, color);
        /* bottom */
        _fill(rx, cenY + (intR - cut) + k, rw, 1, color);
    }
}


static void _erase_meteor(float cenX, float cenY, uint8_t r)
{
    _fill((int16_t)(cenX - r - 1),
          (int16_t)(cenY - r - 1),
          (int16_t)(r * 2 + 3),
          (int16_t)(r * 2 + 3),
          COLOR_BLACK);
}


void Meteors_Init(MeteorPool *pool)
{
    for (int i = 0; i < MAX_METEORS; i++) {
        pool->meteors[i].active    = 0;
        pool->meteors[i].exploding = 0;
        pool->meteors[i].drawn     = 0;
        pool->meteors[i].hp        = 0;
    }
    volatile uint32_t spin = 0;
    uint32_t t0 = HAL_GetTick();

    while (HAL_GetTick() == t0) {
    	spin++;
    }

    srand(HAL_GetTick() ^ (spin * 2654435761UL));
}


void Meteors_Spawn(MeteorPool *pool)
{
    Meteor *m = NULL;
    for (int i = 0; i < MAX_METEORS; i++) {
        if (!pool->meteors[i].active && !pool->meteors[i].exploding) {
            m = &pool->meteors[i];
            break;
        }
    }
    if (!m) return;

    uint8_t r  = METEOR_RADIUS_MIN +
                 (uint8_t)(rand() % (METEOR_RADIUS_MAX - METEOR_RADIUS_MIN + 1));
    int8_t  hp = 1 + (int8_t)(((int)(r - METEOR_RADIUS_MIN) * 9)
                               / (METEOR_RADIUS_MAX - METEOR_RADIUS_MIN));

    float sx, sy;
    switch (rand() % 4) {
        case 0:  sx=(float)(rand()%SCREEN_W); sy=-(float)(r+4);            break;
        case 1:  sx=(float)(rand()%SCREEN_W); sy= (float)(SCREEN_H+r+4);   break;
        case 2:  sx=-(float)(r+4);            sy= (float)(rand()%SCREEN_H); break;
        default: sx= (float)(SCREEN_W+r+4);   sy= (float)(rand()%SCREEN_H); break;
    }

    float tx=SCREEN_W/2.0f+(float)((rand()%100)-50);
    float ty=SCREEN_H/2.0f+(float)((rand()%80)-40);
    float ddx=tx-sx, ddy=ty-sy;
    float len=sqrtf(ddx*ddx+ddy*ddy);
    if (len<1.0f) len=1.0f;

    float spd=METEOR_SPEED_MIN+
              ((float)(rand()%1000)/1000.0f)*(METEOR_SPEED_MAX-METEOR_SPEED_MIN);

    m->x=sx; m->y=sy; m->prevX=sx; m->prevY=sy;
    m->dx=(ddx/len)*spd; m->dy=(ddy/len)*spd;
    m->radius=r; m->hp=hp;
    m->active=1; m->exploding=0; m->drawn=0;
}


static void _start_explosion(Meteor *m)
{
    if (m->drawn) _erase_meteor(m->x, m->y, m->radius);
    m->active=0; m->exploding=1; m->drawn=0;
    for (int i = 0; i < MAX_SHARDS; i++) {
        float a = i*(2.0f*3.14159265f/MAX_SHARDS);
        m->shards[i].x     = m->x + cosf(a)*(float)m->radius;
        m->shards[i].y     = m->y + sinf(a)*(float)m->radius;
        m->shards[i].prevX = m->shards[i].x;
        m->shards[i].prevY = m->shards[i].y;
        m->shards[i].dx    = cosf(a)*2.2f;
        m->shards[i].dy    = sinf(a)*2.2f;
        m->shards[i].life  = SHARD_LIFETIME;
    }
}


void Meteors_Update(MeteorPool *pool)
{
    for (int i = 0; i < MAX_METEORS; i++) {
        Meteor *m = &pool->meteors[i];
        if (m->active) {
            m->prevX=m->x; m->prevY=m->y;
            m->x+=m->dx;   m->y+=m->dy;
            if (m->x<-(float)(m->radius+8) || m->x>(float)(SCREEN_W+m->radius+8) ||
                m->y<-(float)(m->radius+8) || m->y>(float)(SCREEN_H+m->radius+8)) {
                if (m->drawn) _erase_meteor(m->prevX, m->prevY, m->radius);
                m->active=0; m->drawn=0;
            }
        }
        if (m->exploding) {
            uint8_t anyAlive=0;
            for (int s=0; s<MAX_SHARDS; s++) {
                Shard *sh=&m->shards[s];
                if (sh->life==0) continue;
                sh->prevX=sh->x; sh->prevY=sh->y;
                sh->x+=sh->dx;   sh->y+=sh->dy;
                sh->life--; anyAlive=1;
            }
            if (!anyAlive) m->exploding=0;
        }
    }
}


void Meteors_Draw(MeteorPool *pool)
{
    for (int i = 0; i < MAX_METEORS; i++) {
        Meteor *m = &pool->meteors[i];

        if (m->active) {
            if (m->drawn) _erase_meteor(m->prevX, m->prevY, m->radius);
            _draw_octagon((int16_t)m->x, (int16_t)m->y, m->radius, METEOR_COLOR);
            m->drawn=1;
        }

        if (m->exploding) {
            for (int s=0; s<MAX_SHARDS; s++) {
                Shard *sh=&m->shards[s];
                _fill((int16_t)(sh->prevX-2),(int16_t)(sh->prevY-2),4,4,COLOR_BLACK);
                if (sh->life==0) continue;
                uint16_t col=(sh->life>SHARD_LIFETIME/2)?ILI9341_YELLOW:METEOR_COLOR;
                _fill((int16_t)(sh->x-2),(int16_t)(sh->y-2),4,4,col);
            }
        }
    }
}


uint8_t Bullets_CheckMeteorCollisions(BulletPool *bpool, MeteorPool *mpool)
{
    uint8_t anyHit=0;
    for (int b=0; b<MAX_BULLETS; b++) {

        Bullet *bullet=&bpool->bullets[b];
        if (!bullet->active) continue;
        for (int m=0; m<MAX_METEORS; m++) {

            Meteor *meteor=&mpool->meteors[m];
            if (!meteor->active) continue;

            float dx=bullet->x-meteor->x, dy=bullet->y-meteor->y;

            if (sqrtf(dx*dx+dy*dy)<=(float)meteor->radius) {
                bullet->active=0; meteor->hp--;
                if (meteor->hp<=0) _start_explosion(meteor);
                anyHit=1; break;
            }
        }
    }
    return anyHit;
}

uint8_t Meteors_CheckShipCollision(MeteorPool *pool,
                                    float shipX, float shipY, float shipRadius)
{
    for (int i=0; i<MAX_METEORS; i++) {
        Meteor *m=&pool->meteors[i];
        if (!m->active) continue;
        float dx=shipX-m->x, dy=shipY-m->y;
        if (sqrtf(dx*dx+dy*dy)<=(float)m->radius+shipRadius) return 1;
    }
    return 0;
}
