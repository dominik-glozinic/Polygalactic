
/*
 * spaceship.h
 */

#ifndef SPACESHIP_H_
#define SPACESHIP_H_

#include <stdint.h>
#include "ili9341.h"

/* ── Screen & ship constants ─────────────────────────────────────── */
#define SCREEN_W      320
#define SCREEN_H      240

#define SHIP_SIZE     12.0f
#define SHIP_SPEED    3.0f

/*
 * Dont forge to calibrate after the swap
 *
 * Might also add acceleration later (if the joysticks arent garbage)
 */
#define JOY_CENTRE    3200
#define JOY_DEADZONE  250
#define JOY_MAX_POS   900.0f   // centre (3200) to max (4095)
#define JOY_MAX_NEG   895.0f  // centre (3200) to min (0)

#define COLOR_BLACK     0x0000
#define COLOR_WHITE     0xFFFF
#define COLOR_CYAN      0x07FF
#define COLOR_RED       0xF800


#define SHIP_MAX_HP        3
#define SHIP_INVULN_FRAMES 90   // Around 1.5s
#define SHIP_FLICKER_RATE  6    // Draw every N frame


typedef struct {
    float    x, y;
    float    prevX, prevY;
    float    angle, prevAngle;
    uint8_t  needsErase;

    uint32_t lastShotTick;


    int8_t   hp;
    uint16_t invulnFrames;
} Spaceship;


void Ship_Init  (Spaceship *ship);
void Ship_Update(Spaceship *ship, uint32_t *adcBuffer);
void Ship_Draw  (Spaceship *ship);

/* Call once per frame AFTER collision check.
 * Returns 1 if the ship just died (hp reached 0) -> DONT FORGET */
uint8_t Ship_TakeDamage(Spaceship *ship);

#endif


