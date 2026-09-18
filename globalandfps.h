#ifndef GLOBALANDFPS_H
#define GLOBALANDFPS_H

#include <raylib.h>

// ============================================================
//  EL H mas importante de todo, por eso tiene las globales:
// ============================================================
#define SCREEN_WIDTH  960
#define SCREEN_HEIGHT 540
#define FPS_SPEED     60
#define FIXED_DT      (1.0f / (float)FPS_SPEED)

// ============================================================
//  Fixed Timestep
// ============================================================
static float _fixedAccumulator = 0.0f;
int contadorDeFrames = 1;

inline void tickAccumulator() {
    _fixedAccumulator += GetFrameTime();
}

inline bool hasPendingTicks() {
    return _fixedAccumulator >= FIXED_DT;
}

inline void cerrarCicloWhile() {
    _fixedAccumulator -= FIXED_DT;
    if (contadorDeFrames >= FPS_SPEED) contadorDeFrames = 1;
    else contadorDeFrames++;
}

// ============================================================
//  mostrarFPS
// ============================================================
inline void mostrarFPS(int x, int y) {
    DrawText(TextFormat("FPS: %d", GetFPS()), x, y, 10, WHITE);
    DrawText(TextFormat("Frames: %d", contadorDeFrames), x, y + 12, 10, YELLOW);
}

// ============================================================
//  initAudio
// ============================================================
inline void initAudio() {
    InitAudioDevice();
}

// ============================================================
//  initFPSManager — conservado por compatibilidad, no-op.
// ============================================================
inline void initFPSManager() {}

#endif // GLOBALANDFPS_H