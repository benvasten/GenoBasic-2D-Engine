#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

// ============================================================
// PLAYERCONTROLLER.H  
// ============================================================

#include "raylib.h"
#include "Actor.h"
#include "inputmanager.h"
#include "ActorManager.h"
#include "CollisionActors.h"
#include "TileMap.h"


class PlayerController {
public:
    Actor* actor;
    int    playerID;

    PlayerController(Actor* a, int id) : actor(a), playerID(id) {}

    void update(inputs& in,
        const TileMap& map,
        std::vector<Actor*>& allActors,
        Sound jumpSnd,
        Sound dashSnd)
    {
        if (!actor || !actor->alive) return;

        // ============================================================
        // 1. APUNTADO
        // ============================================================
        int aimX = 0, aimY = 0;

        if (in.left.down)  aimX = -1;
        else if (in.right.down) aimX = 1;
        if (in.up.down)    aimY = -1;
        else if (in.down.down)  aimY = 1;

        if (aimX == 0) actor->crouchMoveLock = false;

        actor->isDrifting = (actor->vx < 0 && aimX > 0) ||
            (actor->vx > 0 && aimX < 0);

        // ============================================================
        // 2. AGACHARSE
        // ============================================================
        bool isForcedCrouch = false;

        if (in.down.down && actor->isGrounded) {
            actor->crouch();
        }
        else if (actor->isCrouching) {
            if (canStandUpSafe(actor, map, &allActors[0], (int)allActors.size()))
                actor->standUp();
            else
                isForcedCrouch = true;
        }

        actor->setAim(aimX, aimY);

        // ============================================================
        // 3. MOVIMIENTO HORIZONTAL
        //    btnX → SOLO activa turboRunning (más velocidad máxima).
        //    NO toca justDash ni vx directamente.
        //    El movimiento real siempre viene de aimX.
        // ============================================================
        if (!actor->useInertia) {
            actor->setMovingInput(aimX);
        }
        else {
            // turboRunning amplía maxSpeed SOLO cuando hay dirección activa.
            // Si aimX == 0, forzamos turboRunning=false para que la inercia
            // frene a velocidad normal y no quede vx residual elevado.
            actor->turboRunning = in.btnX.down && (aimX != 0);

            if (actor->isCrouching) {
                actor->setMovingInput(isForcedCrouch ? aimX : 0);
            }
            else {
                actor->setMovingInput(aimX);
            }
        }

        // ============================================================
        // 4. DASH  (btnY)
        //    Requiere aimX != 0: sin dirección no hay dash.
        //    Así dash y turbo (btnX) no se cruzan nunca:
        //      btnX → velocidad alta en la dirección que ya tienes
        //      btnY → impulso puntual en la dirección que apuntas
        // ============================================================
        if (actor->isGrounded) actor->hasDashedInAir = false;

        bool canDash = actor->isGrounded || !actor->hasDashedInAir;

        if (in.btnY.pressed && aimX != 0 &&          // << requiere dirección
            !actor->isCrouching && !actor->justDash && canDash)
        {
            if (dashSnd.frameCount > 0) PlaySound(dashSnd);
            actor->justDash = true;
            if (!actor->isGrounded) actor->hasDashedInAir = true;
        }

        // ============================================================
        // 5. FIREBALL  (btnB, solo Mario de fuego)
        // ============================================================
        if (in.btnB.pressed && actor->animPrefix == "FIRE_") {
            actor->wantsToShootFireball = true;
        }

        // ============================================================
        // 6. SALTO  (btnA)
        // ============================================================
        if (in.btnA.pressed && !actor->isCrouching) {
            if (actor->canJump && actor->isGrounded) {
                if (jumpSnd.frameCount > 0) PlaySound(jumpSnd);
            }
            actor->jumpStart();
        }
        else if (in.btnA.down) { actor->jumpHold(); }
        else if (in.btnA.released) { actor->jumpStop(); }

        // ============================================================
        // 7. ANCLA  (lb)
        // ============================================================
        actor->isAnchored = in.lb.down;
        if (in.lb.down && actor->isGrounded) actor->vx = 0;

        // ============================================================
        // DEBUG TEMPORAL con F3: transformación automatica sin powerup: no flower
        // ============================================================
        if (in.f3.pressed) {
            if (actor->animPrefix == "SMALL_") {
                actor->animPrefix = "BIG_";
                actor->changePhysicalSize(25, 50);
            }
            else if (actor->animPrefix == "BIG_") {
                actor->animPrefix = "SMALL_";
                actor->changePhysicalSize(25, 25);
            }
        }

        // ============================================================
        // CEREBRO VISUAL
        //
        //   IMPORTANTE: nunca llames playAnimation() con nombres que
        //   no existan en ActorFactory. Si el nombre no existe,
        //   playAnimation() deja el render en estado indefinido
        //   lo que genera parpadeo o actor invisible.
        // ============================================================
        const std::string& px = actor->animPrefix;

        if (actor->isCrouching) {
            actor->playAnimation(px + "IDLE");
        }
        else if (actor->justDash) {
            actor->playAnimation(px + "RUN");
        }
        else if (actor->currentJumpFrames != 0 || actor->vy != 0) {
            // Usamos IDLE en lugar de JUMP (no registrado) para evitar
            // estado inválido que causa que desaparezca player al aterrizar.
            actor->playAnimation(px + "JUMP");
        }
        else if (actor->isGrounded) {
            if (actor->vx == 0) {
                //SMALL_IDLE / BIG_IDLE
                actor->playAnimation(px + "IDLE");
            }
            else if (actor->vx == 0) {
                if (actor->isShootingHold) {
                    if (actor->isAnchored) {
                        actor->playAnimation(actor->animPrefix + "SHOOT_ANCHOR");
                    }
                    else {
                        actor->playAnimation(actor->animPrefix + "SHOOT_IDLE");
                    }
                }
                else {
                    actor->playAnimation(actor->animPrefix + "IDLE");
                }
            }
            else {
                if (actor->isShootingHold || actor->justShot) {
                    actor->playAnimation(actor->animPrefix + "RUN_SHOOT");
                }
                else if (actor->turboRunning) {
                    if (actor->isDrifting) {
                        actor->playAnimation(actor->animPrefix + "DRIFTING");
                    }
                    else {
                        actor->playAnimation(actor->animPrefix + "TURBORUN");
                    }
                }
                else if (actor->isDrifting) {
                    actor->playAnimation(actor->animPrefix + "DRIFTING");
                }
                else {
                    actor->playAnimation(actor->animPrefix + "RUN");
                }
            }
        }
        // Si ninguna rama aplica (transición aún no resuelta por la física),
        // NO llamamos a playAnimation() → el sistema mantiene el último
        // frame válido en lugar de pedir un nombre inexistente.
    }
};

#endif // PLAYERCONTROLLER_H