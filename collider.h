#ifndef COLLIDER_H
#define COLLIDER_H

// ============================================================
// COLLIDER.H
// ============================================================

#include "raylib.h"

enum ColliderType {
    COLLIDER_AABB,
    COLLIDER_SPHERE
};

struct SphereCollider {
    float cx, cy;
    float radius;
    float prevCx, prevCy;

    SphereCollider()
        : cx(0), cy(0), radius(0), prevCx(0), prevCy(0) {}
};

class Collider {
public:
    float x, y;
    int   w, h;

    Collider(float x_, float y_, int w_, int h_)
        : x(x_), y(y_), w(w_), h(h_) {}

    Collider()
        : x(10), y(10), w(32), h(32) {}

    void draw(Color color) const {
        DrawRectangleLines((int)x, (int)y, w, h, color);
    }

    void draw_to_cam(int scrollX, int scrollY, Color color) const {
        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();

        if (x - scrollX + w < 0 || x - scrollX > screenW ||
            y - scrollY + h < 0 || y - scrollY > screenH) return;

        DrawRectangleLines(
            (int)x - scrollX,
            (int)y - scrollY,
            w, h,
            color
        );
    }
};

// ============================================================
// Overlap puro (sin Actor)
// ============================================================

inline bool checkAABBOverlap(float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2)
{
    return (x1 < x2 + w2 &&
        x1 + w1 > x2 &&
        y1 < y2 + h2 &&
        y1 + h1 > y2);
}

inline bool checkSphereVsSphere(float cx1, float cy1, float r1,
    float cx2, float cy2, float r2)
{
    float dx = cx1 - cx2;
    float dy = cy1 - cy2;
    float rSum = r1 + r2;
    return (dx * dx + dy * dy) < (rSum * rSum);
}

inline bool checkSphereVsAABB(float cx, float cy, float radius,
    float ax, float ay, float aw, float ah)
{
    float closestX = cx;
    if (cx < ax)       closestX = ax;
    else if (cx > ax + aw) closestX = ax + aw;

    float closestY = cy;
    if (cy < ay)       closestY = ay;
    else if (cy > ay + ah) closestY = ay + ah;

    float dx = cx - closestX;
    float dy = cy - closestY;
    return (dx * dx + dy * dy) < (radius * radius);
}

inline void sphereToAABB(float cx, float cy, float radius,
    float& outX, float& outY, int& outW, int& outH)
{
    float diameter = radius * 2.0f;
    outX = cx - radius;
    outY = cy - radius;
    outW = (int)diameter;
    outH = (int)diameter;
}

#endif // COLLIDER_H
