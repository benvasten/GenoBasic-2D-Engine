#ifndef VFX_H
#define VFX_H

// ============================================================
// VFX.H
// ============================================================

#include "raylib.h"
#include "Actor.h"    // VFXMovement

class VFX {
public:
    float     x, y;
    Texture2D texture;      // No poseída — ver nota arriba
    int       frameWidth;
    int       visualH;
    int       timeInterval;
    int       numFrames;

    VFXMovement movementType;
    float       vx, vy;

    int  animTimer;
    int  currentFrameIndex;
    bool isFinished;

    VFX(float x_, float y_,
        Texture2D tex,
        int fWidth, int vHeight,
        int tInterval, int nFrames,
        VFXMovement moveType)
        : x(x_), y(y_),
        texture(tex),
        frameWidth(fWidth), visualH(vHeight),
        timeInterval(tInterval), numFrames(nFrames),
        movementType(moveType),
        vx(0.0f), vy(0.0f),
        animTimer(0), currentFrameIndex(0), isFinished(false)
    {
        if (movementType == VFX_PARABOLA) {
            vy = -5.0f;
            vx = 1.5f;
        }
        else if (movementType == VFX_UP_AND_DOWN) {
            vy = -5.0f;
        }
    }

    // ============================================================
    // UPDATE — sin cambios funcionales
    // ============================================================
    void update() {
        if (isFinished) return;

        // Física del VFX
        if (movementType == VFX_FALL_DOWN) {
            y += 6.0f;
        }
        else if (movementType == VFX_PARABOLA ||
            movementType == VFX_UP_AND_DOWN) {
            x += vx;
            y += vy;
            vy += 0.9f;   // Gravedad simulada
            if (y > 1000.0f) isFinished = true;
        }

        animTimer++;
        if (animTimer >= timeInterval) {
            animTimer = 0;
            currentFrameIndex++;
            if (currentFrameIndex >= numFrames)
                isFinished = true;
        }
    }

    // ============================================================
    // DRAW
    // ============================================================
    void draw(int scrollX, int scrollY) const {
        if (isFinished || texture.id == 0) return;

        // Ancla centro-bottom (igual que en el original)
        int drawX = (int)x - (frameWidth / 2) - scrollX;
        int drawY = (int)y - visualH - scrollY;

        // Rectángulo fuente: frame actual del spritesheet
        Rectangle src = {
            (float)(currentFrameIndex * frameWidth),
            0.0f,
            (float)frameWidth,
            (float)visualH
        };

        // Rectángulo destino
        Rectangle dst = {
            (float)drawX,
            (float)drawY,
            (float)frameWidth,
            (float)visualH
        };

        DrawTexturePro(texture, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
    }
};

#endif // VFX_H