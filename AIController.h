#ifndef AICONTROLLER_H
#define AICONTROLLER_H

// ============================================================
// AICONTROLLER.H
// ============================================================

#include "raylib.h"
#include "Actor.h"
#include "ActorManager.h"
#include "Camera.h"       // GameCamera
#include "TileMap.h"
#include <cmath>

// ============================================================
// ENUM: comportamientos de IA
// ============================================================
enum EnemyBehavior {
    STATIC_SHOOTER,
    PATROL_SHOOTER,
    JUMP_MEDUSA,
    GOOMBA_INDEFENSO,
    BLOCK_BEH_MUSHROOM,
    BLOCK_BEH_BRICKS,
    STATIC_PIPE,
    MUSHROOM_WALK,
    MUSHROOM_EMERGING,
    BLOCK_BEH_COIN,
    BLOCK_BEH_MULTICOIN,
    COIN_EMERGING,
    COIN_IDLE,
    BLOCK_BEH_FLOWER,
    FLOWER_EMERGING,
    FIREBALL_BEH,
    GREEN_KOOPA_WALK,
    RED_KOOPA_WALK,
    KOOPA_SHELL_IDLE,
    KOOPA_SHELL_SLIDE,
    STAR_BOUNCING
};


class AIController {
public:
    Actor* enemy;
    EnemyBehavior behavior;
    int           stateTimer;
    int           direction;    // 1 = derecha, -1 = izquierda

    AIController(Actor* e, EnemyBehavior b)
        : enemy(e), behavior(b), stateTimer(0), direction(-1) {}

    // ============================================================
    // UPDATE
    // ============================================================
    void update(Actor* targetPlayer,
        ActorManager& manager,
        Sound shootSnd,
        Sound coinSnd,
        Sound powerUpEmergingSound,
        Sound brickShatterSound,
        Sound brickMuffSound,
        const GameCamera& cam,
        const TileMap& tilemap)
    {
        if (!enemy || !enemy->alive) return;

        // ── Sistema de activación (culling lógico) ──────────────
        if (!enemy->isAwake) {
            if (enemy->isStored) {
                stateTimer = 0;   // Congelado dentro del bloque
                return;
            }
            // Despertar cuando la cámara se acerca
            if (enemy->x < cam.x + cam.screenW + 64) enemy->isAwake = true;
            else return;
        }

        stateTimer++;

        // ============================================================
        // PATRULLERO DISPARADOR
        // ============================================================
        if (behavior == PATROL_SHOOTER) {
            if (stateTimer % 120 == 0) direction *= -1;
            enemy->setMovingInput(direction);

            if (targetPlayer && targetPlayer->alive)
                enemy->lookingLeft = (targetPlayer->x < enemy->x);

            int shootCycle = stateTimer % 90;
            if (shootCycle == 0) {
                enemy->setAim(0, 0);
                Bullet* b = enemy->shoot("files//cuphead//cupheadBullet.bmp");
                if (b) {
                    manager.addEnemyBullet(b);
                    if (shootSnd.frameCount > 0) PlaySound(shootSnd);
                }
            }
            if (shootCycle >= 0 && shootCycle < 25) enemy->playAnimation("SHOOT");
            else                                     enemy->playAnimation("IDLE");
        }

        // ============================================================
        // BLOQUES (Mushroom, Brick, Coin, Multicoin, Flower)
        // ============================================================
        else if (behavior == BLOCK_BEH_MUSHROOM || behavior == BLOCK_BEH_BRICKS ||
            behavior == BLOCK_BEH_COIN || behavior == BLOCK_BEH_MULTICOIN ||
            behavior == BLOCK_BEH_FLOWER)
        {
            enemy->vx = 0;

            // Animación según estado
            if (enemy->beenHitAsBlock == NOTHIT || enemy->beenHitAsBlock == BEINGHIT) {
                if (behavior == BLOCK_BEH_BRICKS) enemy->playAnimation("BRICK_NOTHIT");
                else                              enemy->playAnimation("MUSH_NOTHIT");
            }
            else {
                if (behavior == BLOCK_BEH_BRICKS) enemy->playAnimation("BRICK_HIT");
                else                              enemy->playAnimation("MUSH_HIT");
            }

            // Timer multimoneda
            if (behavior == BLOCK_BEH_MULTICOIN && enemy->multiTimer > 0)
                enemy->multiTimer--;

            // Física de golpe (rebote)
            if (enemy->beenHitAsBlock == BEINGHIT) {
                enemy->beenHitCount++;

                // Frame 1: dar a luz al ítem almacenado
                if (enemy->beenHitCount == 1) {
                    if (behavior == BLOCK_BEH_COIN && enemy->storedItem != NULL) {
                        enemy->storedItem->isHidden = false;
                        enemy->storedItem->isStored = false;
                        enemy->storedItem->isAwake = true;
                        enemy->storedItem->y = enemy->originalBlock_Y;
                        enemy->storedItem->syncCollider();
                        enemy->storedItem->vy = -10.0f;
                        enemy->storedItem = NULL;
                        if (coinSnd.frameCount > 0) PlaySound(coinSnd);
                    }
                    else if (behavior == BLOCK_BEH_MULTICOIN && enemy->storedItem != NULL) {
                        if (enemy->multiTimer == -1) enemy->multiTimer = 300;
                        if (enemy->multiTimer > 0) {
                            enemy->storedItem->isHidden = false;
                            enemy->storedItem->isStored = false;
                            enemy->storedItem->isAwake = true;
                            enemy->storedItem->y = enemy->originalBlock_Y;
                            enemy->storedItem->syncCollider();
                            enemy->storedItem->vy = -8.0f;
                            if (coinSnd.frameCount > 0) PlaySound(coinSnd);
                        }
                    }

                    // Sonido sordo del bloque (todos excepto MUSHROOM al primer frame)
                    if (!(behavior == BLOCK_BEH_MUSHROOM)) {
                        if (brickMuffSound.frameCount > 0) PlaySound(brickMuffSound);
                    }
                    enemy->vy = -3.0f;
                    enemy->isLavaFloored = true;
                }
                // Frame 4: destrucción del ladrillo si Mario es grande
                else if (enemy->beenHitCount == 4) {
                    if (behavior == BLOCK_BEH_BRICKS && enemy->hitByBigMario) {
                        if (brickShatterSound.frameCount > 0) PlaySound(brickShatterSound);
                        enemy->alive = false;
                        return;
                    }
                }
                else if (enemy->beenHitCount >= 4 && enemy->beenHitCount < 6) { /* espera en el pico */ }
                else if (enemy->beenHitCount >= 6 && enemy->beenHitCount < 10) {
                    enemy->vy = 3.0f;   // Cae
                }
                else if (enemy->beenHitCount >= 10) {
                    // Fin del rebote: volver a posición original
                    enemy->beenHitCount = 0;
                    enemy->vy = 0;
                    enemy->y = enemy->originalBlock_Y;
                    enemy->isLavaFloored = false;

                    // Nacimiento retrasado del champiñón (espera a que el bloque baje)
                    if (behavior == BLOCK_BEH_MUSHROOM && enemy->storedItem != NULL) {
                        float blockCenter = enemy->collider.x + (enemy->collider.w / 2.0f);
                        enemy->storedItem->targetDirection =
                            (enemy->lastHitByX < blockCenter) ? 1 : -1;

                        // Champiñón (1) y Flor (2) mutan según estado de Mario
                        // 1-Up (4) y Estrella (5) NO mutan
                        if (enemy->storedItem->powerUpType == 1 ||
                            enemy->storedItem->powerUpType == 2)
                        {
                            if (enemy->hitByBigMario) {
                                enemy->storedItem->powerUpType = 2;
                                enemy->storedItem->playAnimation("FLOWER_IDLE");
                            }
                            else {
                                enemy->storedItem->powerUpType = 1;
                                enemy->storedItem->playAnimation("IDLE");
                            }
                        }

                        enemy->storedItem->isHidden = false;
                        enemy->storedItem->isStored = false;
                        enemy->storedItem->isAwake = true;
                        enemy->storedItem = NULL;
                    }

                    // Estado del bloque tras el rebote
                    if (behavior == BLOCK_BEH_MULTICOIN && enemy->multiTimer > 0) {
                        enemy->beenHitAsBlock = NOTHIT;      // Multimoneda sigue activo
                    }
                    else if (behavior == BLOCK_BEH_BRICKS) {
                        enemy->beenHitAsBlock = NOTHIT;      // Ladrillo: golpeable infinitas veces
                    }
                    else {
                        enemy->beenHitAsBlock = ALREADYHIT;  // Champiñón/moneda única: marrón
                    }
                }
            }
        }

        // ============================================================
        // TIRADOR ESTÁTICO
        // ============================================================
        else if (behavior == STATIC_SHOOTER) {
            enemy->vx = 0;
            if (targetPlayer && targetPlayer->alive)
                enemy->lookingLeft = (targetPlayer->x < enemy->x);

            if (stateTimer % 60 == 0) {
                enemy->setAim(0, 0);
                Bullet* b = enemy->shoot("files//cuphead//cupheadBullet.bmp");
                if (b) manager.addEnemyBullet(b);
            }
        }

        // ============================================================
        // POWER-UP SALIENDO DEL BLOQUE
        // ============================================================
        else if (behavior == MUSHROOM_EMERGING) {
            enemy->affectedByGravity = false;

            if (stateTimer == 1) {
                if (powerUpEmergingSound.frameCount > 0) PlaySound(powerUpEmergingSound);
            }

            if (stateTimer < 68) {
                enemy->vy = -0.5f;   // Sube lentamente
            }
            else {
                enemy->vy = 0;
                // Champiñón y 1-Up: comienzan a caminar
                if (enemy->powerUpType == 1 || enemy->powerUpType == 4) {
                    behavior = MUSHROOM_WALK;
                    enemy->affectedByGravity = true;
                    direction = enemy->targetDirection;
                    stateTimer = 0;
                }
                // Estrella: comienza a rebotar
                else if (enemy->powerUpType == 5) {
                    behavior = STAR_BOUNCING;
                    enemy->affectedByGravity = true;
                    direction = enemy->targetDirection;
                    stateTimer = 0;
                }
                // Flor (tipo 2): se queda flotando con vy=0, sin más lógica
            }
        }

        // ============================================================
        // MONEDA SALIENDO DEL BLOQUE
        // ============================================================
        else if (behavior == COIN_EMERGING) {
            enemy->affectedByGravity = false;
            enemy->vy += 0.5f;   // Gravedad simulada manualmente

            if (enemy->vy > 0 && stateTimer > 15) {
                enemy->isHidden = true;
                enemy->isStored = true;
                enemy->isAwake = false;
                enemy->vy = 0;
            }
            enemy->playAnimation("IDLE");
        }

        // ============================================================
        // MONEDA QUIETA EN EL MAPA
        // ============================================================
        else if (behavior == COIN_IDLE) {
            enemy->playAnimation("IDLE");
        }

        // ============================================================
        // FLOR SALIENDO DEL BLOQUE
        // ============================================================
        else if (behavior == FLOWER_EMERGING) {
            enemy->affectedByGravity = false;
            if (stateTimer < 68) enemy->vy = -0.5f;
            else                 enemy->vy = 0.0f;
            enemy->playAnimation("IDLE");
        }

        // ============================================================
        // TUBO ESTÁTICO
        // ============================================================
        else if (behavior == STATIC_PIPE) {
            enemy->vx = 0;
            enemy->vy = 0;
        }

        // ============================================================
        // GOOMBA
        // ============================================================
        else if (behavior == GOOMBA_INDEFENSO) {
            enemy->affectedByGravity = true;
            if (enemy->vx == 0 && stateTimer > 1) direction *= -1;
            enemy->vx = 0.8f * direction;
            enemy->lookingLeft = (direction == -1);
        }

        // ============================================================
        // CHAMPIÑÓN (power-up caminando)
        // ============================================================
        else if (behavior == MUSHROOM_WALK) {
            enemy->affectedByGravity = true;
            if (enemy->vx == 0 && stateTimer > 1) direction *= -1;
            enemy->vx = 1.2f * direction;
            enemy->playAnimation("IDLE");
        }

        // ============================================================
        // ESTRELLA DE INVENCIBILIDAD
        // ============================================================
        else if (behavior == STAR_BOUNCING) {
            enemy->affectedByGravity = true;
            if (enemy->vx == 0 && stateTimer > 1) direction *= -1;
            enemy->vx = 2.0f * direction;
            if (enemy->isGrounded) {
                enemy->vy = -6.0f;
                enemy->isGrounded = false;
            }
            enemy->playAnimation("IDLE");
        }

        // ============================================================
        // BOLA DE FUEGO
        // ============================================================
        else if (behavior == FIREBALL_BEH) {
            // Destruir si sale de la cámara
            if (enemy->x < cam.x - 32 ||
                enemy->x > cam.x + cam.screenW + 32 ||
                enemy->y > cam.y + cam.screenH + 32)
            {
                enemy->alive = false;
                return;
            }

            enemy->affectedByGravity = true;

            // Si las físicas la detuvieron horizontalmente (muro sólido), estalla
            if (enemy->vx == 0 && stateTimer > 1) {
                enemy->alive = false;
                return;
            }

            if (stateTimer == 1) direction = enemy->targetDirection;
            enemy->vx = 6.0f * direction;

            // Rebote al tocar el suelo
            if (enemy->isGrounded) {
                enemy->vy = -6.0f;
                enemy->isGrounded = false;
            }
            enemy->playAnimation("IDLE");
        }

        // ============================================================
        // KOOPA VERDE (camina y cae en huecos)
        // ============================================================
        else if (behavior == GREEN_KOOPA_WALK) {
            enemy->affectedByGravity = true;

            // Al ser pisado (vidas == 1): se convierte en caparazón
            if (enemy->vidas == 1) {
                behavior = KOOPA_SHELL_IDLE;
                enemy->isShell = true;
                enemy->dealsDamage = false;
                enemy->changePhysicalSize(30, 30);
                enemy->vx = 0;
                return;
            }

            if (enemy->vx == 0 && stateTimer > 1) direction *= -1;
            enemy->vx = 0.8f * direction;
            enemy->lookingLeft = (direction == -1);
            enemy->playAnimation("WALK");
        }

        // ============================================================
        // KOOPA ROJO (no cae en huecos)
        // ============================================================
        else if (behavior == RED_KOOPA_WALK) {
            enemy->affectedByGravity = true;

            if (enemy->vidas == 1) {
                behavior = KOOPA_SHELL_IDLE;
                enemy->isShell = true;
                enemy->dealsDamage = false;
                enemy->changePhysicalSize(30, 30);
                enemy->vx = 0;
                return;
            }

            // Sensor de borde: palpa delante y abajo para detectar hueco
            if (enemy->isGrounded) {
                float sensorX = (direction == 1)
                    ? (enemy->collider.x + enemy->collider.w + 2)
                    : (enemy->collider.x - 2);
                float sensorY = enemy->collider.y + enemy->collider.h + 2;
                int   tileX = (int)(sensorX / TILE_SIZE);
                int   tileY = (int)(sensorY / TILE_SIZE);

                bool hasFloor = tilemap.isSolid(tileX, tileY);

                if (!hasFloor) {
                    const std::vector<Actor*>& obstacles = manager.getObstacles();
                    for (size_t i = 0; i < obstacles.size() && !hasFloor; i++) {
                        Actor* obs = obstacles[i];
                        if (obs->alive && obs->isSolid &&
                            sensorX >= obs->collider.x &&
                            sensorX <= (obs->collider.x + obs->collider.w) &&
                            sensorY >= obs->collider.y &&
                            sensorY <= (obs->collider.y + obs->collider.h))
                            hasFloor = true;
                    }
                }

                if (!hasFloor) {
                    const std::vector<Actor*>& platforms = manager.getPlatforms();
                    for (size_t i = 0; i < platforms.size() && !hasFloor; i++) {
                        Actor* plat = platforms[i];
                        if (plat->alive && plat->isSolid &&
                            sensorX >= plat->collider.x &&
                            sensorX <= (plat->collider.x + plat->collider.w) &&
                            sensorY >= plat->collider.y &&
                            sensorY <= (plat->collider.y + plat->collider.h))
                            hasFloor = true;
                    }
                }

                if (!hasFloor) direction *= -1;
            }

            if (enemy->vx == 0 && stateTimer > 1) direction *= -1;
            enemy->vx = 0.8f * direction;
            enemy->lookingLeft = (direction == -1);
            enemy->playAnimation("WALK");
        }

        // ============================================================
        // CAPARAZÓN QUIETO
        // ============================================================
        else if (behavior == KOOPA_SHELL_IDLE) {
            enemy->affectedByGravity = true;

            // Pisado estando quieto (vidas == 0): sale disparado
            if (enemy->vidas <= 0) {
                behavior = KOOPA_SHELL_SLIDE;
                enemy->vidas = 1;
                enemy->dealsDamage = true;

                if (targetPlayer) {
                    float shellCenter = enemy->collider.x + (enemy->collider.w / 2.0f);
                    float playerCenter = targetPlayer->collider.x + (targetPlayer->collider.w / 2.0f);
                    direction = (playerCenter < shellCenter) ? -1 : 1;
                }
                enemy->vx = 0;

                if (enemy->causeOfDeath == DEATH_FLIPPED) enemy->playAnimation("SHELL_FLIPPED");
                else                                       enemy->playAnimation("SHELL_IDLE");
                return;
            }

            // Pateado lateralmente por Mario (ActorManager le asignó vx)
            if (enemy->dealsDamage && enemy->vx != 0) {
                behavior = KOOPA_SHELL_SLIDE;
                direction = (enemy->vx > 0) ? 1 : -1;
                return;
            }

            enemy->vx = 0;
            enemy->playAnimation("SHELL_IDLE");
        }

        // ============================================================
        // CAPARAZÓN DESLIZÁNDOSE
        // ============================================================
        else if (behavior == KOOPA_SHELL_SLIDE) {
            enemy->affectedByGravity = true;

            // Consumir el período de gracia post-patada
            if (enemy->kickGracePeriod > 0) enemy->kickGracePeriod--;

            // Pisado de nuevo (vidas == 0): vuelve a caparazón quieto
            if (enemy->vidas <= 0) {
                enemy->vidas = 1;
                behavior = KOOPA_SHELL_IDLE;
                enemy->dealsDamage = false;
                enemy->kickGracePeriod = 0;   // limpiar al detenerlo
                enemy->vx = 0;
                return;
            }

            // Rebote en paredes (las físicas lo paran con vx=0)
            if (enemy->vx == 0 && stateTimer > 1) direction *= -1;
            enemy->vx = 6.0f * direction;

            if (enemy->causeOfDeath == DEATH_FLIPPED) enemy->playAnimation("SHELL_FLIPPED");
            else                                       enemy->playAnimation("SHELL_SLIDE");
        }

        // ============================================================
        // JUMP MEDUSA
        // ============================================================
        else if (behavior == JUMP_MEDUSA) {
            enemy->affectedByGravity = true;

            if (targetPlayer && targetPlayer->alive)
                enemy->lookingLeft = (targetPlayer->x < enemy->x);

            int cycle = stateTimer % 120;

            if (cycle == 0) {
                if (enemy->isGrounded) {
                    if (targetPlayer && targetPlayer->alive)
                        enemy->vx = (targetPlayer->x < enemy->x) ? -3.0f : 3.0f;
                    // jumpStart() ya no recibe sonido; JUMP_MEDUSA no tiene SFX de salto
                    enemy->jumpStart();
                }
            }
            else if (cycle > 0 && cycle <= 15) {
                enemy->jumpHold();
            }
            else if (cycle == 16) {
                enemy->jumpStop();
            }

            if (cycle > 20 && enemy->isGrounded) enemy->vx = 0;
        }
    }
};

#endif // AICONTROLLER_H