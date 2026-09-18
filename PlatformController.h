#ifndef PLATFORMCONTROLLER_H
#define PLATFORMCONTROLLER_H

// ============================================================
// PLATFORMCONTROLLER.H:

// comportamientos de plataformas movibles o no
// ============================================================

#include "Actor.h"

enum PlatformType {
    PING_PONG_X,   // Va de izquierda a derecha
    PING_PONG_Y,   // Va de arriba a abajo
    FALLING        // Cae al ser pisada
};

class PlatformController {
public:
    Actor* platform;
    PlatformType type;
    int          timer;
    int          limit;      // Frames de viaje antes de dar la vuelta
    float        speed;
    int          direction;  // 1 o -1

    PlatformController(Actor* p, PlatformType t, int travelLimit, float spd)
        : platform(p), type(t), timer(0), limit(travelLimit),
        speed(spd), direction(1) {}

    void update() {
        if (!platform) return;

        timer++;
        if (timer >= limit) {
            direction *= -1;
            timer = 0;
        }

        if (type == PING_PONG_X) {
            platform->vx = speed * direction;
            platform->vy = 0.0f;
        }
        else if (type == PING_PONG_Y) {
            platform->vx = 0.0f;
            platform->vy = speed * direction;
        }
    }
};

#endif // PLATFORMCONTROLLER_H