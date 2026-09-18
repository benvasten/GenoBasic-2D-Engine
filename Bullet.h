#ifndef BULLET_H
#define BULLET_H

// ============================================================
// BULLET.H
// ============================================================

#include "raylib.h"
#include "Collider.h"
#include <cmath>    // atan2, fabs

class Bullet {
public:
    float    x, y;
    int      w, h;
    float    vx, vy;
    Collider collider;
    Texture2D texture;
    bool     alive;
    float    angle;     // Grados (0° = derecha, sentido horario)

    Bullet(float x_, float y_, int w_, int h_,
        const char* spriteRoute, float vx_, float vy_)
        : x(x_), y(y_), w(w_), h(h_),
        vx(vx_), vy(vy_),
        alive(true),
        collider(x_, y_, w_, h_),
        angle(0.0f)
    {
        texture = LoadTexture(spriteRoute);

        // Deducimos angulo segun velocidad
        // Asumiendo que el sprite base apunta a la derecha (como debse ser siempre)
        if (vx > 0 && vy == 0)  angle = 0.0f;        // Derecha
        else if (vx > 0 && vy > 0)   angle = 45.0f;   // Diagonal abajo-derecha
        else if (vx == 0 && vy > 0)  angle = 90.0f;   // Abajo
        else if (vx < 0 && vy > 0)   angle = 135.0f;   // Diagonal abajo-izquierda
        else if (vx < 0 && vy == 0)  angle = 180.0f;   // Izquierda
        else if (vx < 0 && vy < 0)   angle = 225.0f;   // Diagonal arriba-izquierda
        else if (vx == 0 && vy < 0)  angle = 270.0f;   // Arriba
        else if (vx > 0 && vy < 0)   angle = 315.0f;   // Diagonal arriba-derecha
        else                          angle = 0.0f;
    }

    // Bullet es dueño de su textura → la descarga al destruirse
    ~Bullet() {
        if (texture.id != 0) {
            UnloadTexture(texture);
            texture = { 0 };
        }
    }

private:
    // No copiar — cada Bullet posee una textura única
    Bullet(const Bullet&);
    Bullet& operator=(const Bullet&);

public:
    void update() {
        x += vx;
        y += vy;
        collider.x = x;
        collider.y = y;
    }

    bool isOffScreen(int mapWidthPixels, int mapHeightPixels) const {
        return (x + w < 0 || x > mapWidthPixels ||
            y + h < 0 || y > mapHeightPixels);
    }

    // ============================================================
    // DRAW
    // ============================================================
    void draw(int scrollX, int scrollY) const {
        if (texture.id == 0) return;

        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();

        // Culling
        if (x - scrollX + w < 0 || x - scrollX > screenW ||
            y - scrollY + h < 0 || y - scrollY > screenH) return;

        float drawX = x - scrollX;
        float drawY = y - scrollY;

        // Rectángulo fuente: toda la textura
        Rectangle src = {
            0.0f, 0.0f,
            (float)texture.width,
            (float)texture.height
        };

        // Rectángulo destino: mismo tamaño, posicionado en mundo
        Rectangle dst = {
            drawX,
            drawY,
            (float)w,
            (float)h
        };

        // Pivote en el centro del sprite (reproduce rotate_sprite)
        Vector2 origin = { w / 2.0f, h / 2.0f };

        DrawTexturePro(texture, src, dst, origin, angle, WHITE);
    }
};

#endif // BULLET_H