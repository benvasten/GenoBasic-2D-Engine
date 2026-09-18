#ifndef CAMERA_H
#define CAMERA_H

// ============================================================
// CAMERA_H  →  GameCamera
// ============================================================

struct GameCamera {
    float x, y;
    int screenW, screenH;
    int mapW, mapH;     // Límites del mundo en píxeles

    GameCamera(int sw, int sh)
        : x(0), y(0), screenW(sw), screenH(sh), mapW(0), mapH(0) {}

    // Informa al sistema de cámara los límites del nivel cargado
    void setLimits(int mapWidthPixels, int mapHeightPixels) {
        mapW = mapWidthPixels;
        mapH = mapHeightPixels;
    }

    // Centra la cámara en el objetivo y la clampea al mapa
    void update(float targetX, float targetY) {
        x = targetX - (screenW / 2.0f);
        y = targetY - (screenH / 2.0f);

        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x > mapW - screenW) x = (float)(mapW - screenW);
        if (y > mapH - screenH) y = (float)(mapH - screenH);
    }
};

#endif // CAMERA_H