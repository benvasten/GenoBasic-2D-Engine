#ifndef PHYSICSSYSTEM_H
#define PHYSICSSYSTEM_H

#include "Actor.h"

// Constantes mundo físico
#define GRAVITY        0.5f
#define MAX_FALL_SPEED 8.0f


// ============================================================
// GRAVEDAD
// ============================================================

void applyGravity(Actor* actor) {
    if (actor->affectedByGravity) {
        actor->vy += GRAVITY;
        if (actor->vy > MAX_FALL_SPEED)
            actor->vy = MAX_FALL_SPEED;
    }
}

void applyGravityToAll(Actor* actors[], int count) {
    for (int i = 0; i < count; i++)
        applyGravity(actors[i]);
}


// ============================================================
// GROUNDED
// ============================================================

void resetGroundedState(Actor* actors[], int count) {
    for (int i = 0; i < count; i++)
        actors[i]->isGrounded = false;
}

void updateJumpState(Actor* actors[], int count) {
    for (int i = 0; i < count; i++) {
        Actor* a = actors[i];
        if (a->isGrounded) {
            a->canJump = true;
            a->isJumping = false;
            a->currentJumpFrames = 0;
        }
    }
}


// ============================================================
// CARRIERS (plataformas móviles)
// ============================================================

void resetCarryStates(Actor* actors[], int count) {
    for (int i = 0; i < count; i++)
        actors[i]->carriedBy = NULL;
}

void savePreviousPositions(Actor* actors[], int count) {
    for (int i = 0; i < count; i++)
        actors[i]->savePreviousPosition();
}

void applyCarrierMovement(Actor* actors[], int count) {
    for (int i = 0; i < count; i++) {
        Actor* a = actors[i];
        if (a->carriedBy != NULL) {
            float deltaX = a->carriedBy->x - a->carriedBy->prevX;
            float deltaY = a->carriedBy->y - a->carriedBy->prevY;
            a->x += deltaX;
            a->y += deltaY;
            a->syncCollider();
        }
    }
}


// ============================================================
// MOVIMIENTO — separado en X e Y para resolución de colisiones
// ============================================================

void moveCarriers(Actor* actors[], int count) {
    for (int i = 0; i < count; i++)
        if (actors[i]->canCarry) { actors[i]->moveX(); actors[i]->moveY(); }
}

void moveNonCarriers(Actor* actors[], int count) {
    for (int i = 0; i < count; i++)
        if (!actors[i]->canCarry) { actors[i]->moveX(); actors[i]->moveY(); }
}

void moveCarriersX(Actor* actors[], int count) {
    for (int i = 0; i < count; i++)
        if (actors[i]->canCarry) actors[i]->moveX();
}

void moveCarriersY(Actor* actors[], int count) {
    for (int i = 0; i < count; i++)
        if (actors[i]->canCarry) actors[i]->moveY();
}

void moveNonCarriersX(Actor* actors[], int count) {
    for (int i = 0; i < count; i++)
        if (!actors[i]->canCarry) actors[i]->moveX();
}

void moveNonCarriersY(Actor* actors[], int count) {
    for (int i = 0; i < count; i++) {
        if (!actors[i]->canCarry) {
            actors[i]->intendedVy = actors[i]->vy;
            actors[i]->moveY();
        }
    }
}

#endif // PHYSICSSYSTEM_H