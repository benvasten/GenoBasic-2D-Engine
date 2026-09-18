#ifndef COLLISIONTILEMAP_H
#define COLLISIONTILEMAP_H

// ============================================================
// COLLISIONTILEMAP.H
// ============================================================

#include "Actor.h"
#include "TileMap.h"
#include <cmath>


// ============================================================
// COLISIÓN ACTOR VS TILEMAP — EJE X
// ============================================================

void resolveActorVsTilemapX(Actor* actor, const TileMap& tilemap) {

    // Movimiento real del frame: posición actual vs previa
    float effectiveVx = actor->collider.x - actor->prevX;

    int actorLeft = (int)floor(actor->collider.x / TILE_SIZE);
    int actorRight = (int)floor((actor->collider.x + actor->collider.w - 1) / TILE_SIZE);
    int actorTop = (int)floor(actor->collider.y / TILE_SIZE);
    int actorBottom = (int)floor((actor->collider.y + actor->collider.h - 1) / TILE_SIZE);

    if (effectiveVx > 0) {
        // Moviéndose a la derecha
        for (int ty = actorTop; ty <= actorBottom; ty++) {
            if (tilemap.isSolid(actorRight, ty)) {
                actor->x = actorRight * TILE_SIZE - actor->collider.w;
                actor->collider.x = actor->x;
                actor->vx = 0;
                return;
            }
        }
    }
    else if (effectiveVx < 0) {
        // Moviéndose a la izquierda
        for (int ty = actorTop; ty <= actorBottom; ty++) {
            if (tilemap.isSolid(actorLeft, ty)) {
                actor->x = (actorLeft + 1) * TILE_SIZE;
                actor->collider.x = actor->x;
                actor->vx = 0;
                return;
            }
        }
    }
}


// ============================================================
// COLISIÓN ACTOR VS TILEMAP — EJE Y
// ============================================================

void resolveActorVsTilemapY(Actor* actor, const TileMap& tilemap) {

    float nextY = actor->collider.y + actor->vy;

    int actorLeft = (int)floor(actor->collider.x / TILE_SIZE);
    int actorRight = (int)floor((actor->collider.x + actor->collider.w - 1) / TILE_SIZE);
    int actorTop = (int)floor(nextY / TILE_SIZE);
    int actorBottom = (int)floor((nextY + actor->collider.h - 1) / TILE_SIZE);

    if (actor->vy > 0) {
        // Cayendo
        for (int tx = actorLeft; tx <= actorRight; tx++) {
            if (tilemap.isSolid(tx, actorBottom)) {
                //actor->isInvincible = false; <-- borralo porque cancela el trigger invincibility
                actor->y = actorBottom * TILE_SIZE - actor->collider.h;
                actor->collider.y = actor->y;
                actor->vy = 0;
                actor->isGrounded = true;
                return;
            }
        }
    }
    else if (actor->vy < 0) {
        // Subiendo (salto)
        for (int tx = actorLeft; tx <= actorRight; tx++) {
            if (tilemap.isSolid(tx, actorTop)) {
                actor->y = (actorTop + 1) * TILE_SIZE;
                actor->collider.y = actor->y;
                actor->vy = 0;
                return;
            }
        }
    }
}

#endif // COLLISIONTILEMAP_H