/*
 * meteor.h
 */

#ifndef METEOR_H_
#define METEOR_H_

#include <stdint.h>
#include "ili9341.h"
#include "spaceship.h"
#include "bullet.h"

#ifndef SCREEN_W
#define SCREEN_W  320
#endif
#ifndef SCREEN_H
#define SCREEN_H  240
#endif

/* ── Tuning ──────────────────────────────────────────────────────────
 * Keep MAX_METEORS at 3. Each meteor costs 2 FillRectangle calls/frame
 * (erase + draw). More than 3 visibly slows the game on 72 MHz Blue Pill
 * with SPI2 at PCLK/2.
 * ─────────────────────────────────────────────────────────────────── */
#define MAX_METEORS       3
#define METEOR_RADIUS_MIN 8
#define METEOR_RADIUS_MAX 24
#define METEOR_SPEED_MIN  0.7f
#define METEOR_SPEED_MAX  1.6f

/* Explosion: 8 pixel shards flying outward */
#define MAX_SHARDS        8
#define SHARD_LIFETIME    18   /* frames */

typedef struct {
    float   x, y, prevX, prevY;
    float   dx, dy;
    uint8_t life;
} Shard;

typedef struct {
    float   x, y;         /* current centre (float for smooth movement) */
    float   prevX, prevY; /* centre last frame — used for erase          */
    float   dx, dy;
    uint8_t radius;       /* also half the side of the erase/draw rect   */
    int8_t  hp;
    uint8_t active;
    uint8_t exploding;
    uint8_t drawn;        /* 1 once the meteor has been drawn at least once */
    Shard   shards[MAX_SHARDS];
} Meteor;

typedef struct {
    Meteor meteors[MAX_METEORS];
} MeteorPool;

/* Forward-declare so meteor.h doesn't need to include bullet.h */
struct BulletPool;

/* Meteors_Init also calls srand() with jitter entropy */
void    Meteors_Init  (MeteorPool *pool);
void    Meteors_Spawn (MeteorPool *pool);
void    Meteors_Update(MeteorPool *pool);
void    Meteors_Draw  (MeteorPool *pool);

uint8_t Bullets_CheckMeteorCollisions(BulletPool *bpool, MeteorPool *mpool);
uint8_t Meteors_CheckShipCollision   (MeteorPool *pool,
                                       float shipX, float shipY,
                                       float shipRadius);

#endif /* METEOR_H_ */
