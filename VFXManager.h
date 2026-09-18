#ifndef VFXMANAGER_H
#define VFXMANAGER_H

// ============================================================
// VFXMANAGER.H
// ============================================================

#include "raylib.h"
#include <vector>
#include "VFX.h"


class VFXManager {
private:
    std::vector<VFX*> effects;

public:
    VFXManager() {}

    ~VFXManager() {
        for (size_t i = 0; i < effects.size(); i++)
            delete effects[i];
        effects.clear();
    }

    // ============================================================
    // SPAWN — crea un efecto visual puntual
    //   NO es dueño del texture, el texture le pertenece al cache del nivel (deathTexture)
    // ============================================================
    void spawn(float x, float y,
        Texture2D texture,
        int fWidth, int vHeight,
        int tInterval, int nFrames,
        VFXMovement moveType = VFX_STATIC)
    {
        // Guard: textura no válida → no crear efecto
        if (texture.id == 0) return;

        effects.push_back(new VFX(x, y,
            texture,
            fWidth, vHeight,
            tInterval, nFrames,
            moveType));
    }

    // ============================================================
    // UPDATE hace avanzar todos los efectos y elimina los terminados
    // ============================================================
    void update() {
        for (std::vector<VFX*>::iterator it = effects.begin();
            it != effects.end(); )
        {
            (*it)->update();
            if ((*it)->isFinished) {
                delete* it;
                it = effects.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    // ============================================================
    // DRAW ALL (no buffer)
    // ============================================================
    void drawAll(int scrollX, int scrollY) const {
        for (size_t i = 0; i < effects.size(); i++)
            effects[i]->draw(scrollX, scrollY);
    }
};

#endif // VFXMANAGER_H