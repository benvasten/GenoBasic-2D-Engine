#ifndef ACTORFACTORY_H
#define ACTORFACTORY_H

// ============================================================
// ACTORFACTORY.H
// ============================================================

#include "raylib.h"
#include <string>
#include <map>
#include "ActorManager.h"
#include "AIController.h"
#include "PlatformController.h"


class ActorFactory {
public:
    static Actor* spawn(const std::string& type, float x, float y,
        ActorManager& actors,
        std::vector<AIController>& aiList,
        std::vector<PlatformController>& platList,
        std::map<std::string, Texture2D>& sprites)
    {
        EngineColor WHITE_E(255, 255, 255);
        EngineColor RED_E(255, 0, 0);
        EngineColor GREEN_E(0, 255, 0);
        EngineColor BLUE_E(0, 0, 255);
        EngineColor PINK_E(255, 20, 147);

        // ============================================================
        // PLATAFORMAS
        // ============================================================
        if (type == "PLATFORM_NORMAL") {
            Actor* plat = actors.createPlatform(x, y, 96, 24, GREEN_E, "Platform", 96, 37);
            platList.push_back(PlatformController(plat, PING_PONG_X, 120, 2.0f));
            Texture2D t = spr(sprites, "PLAT_1");
            if (t.id) plat->addAnimation("IDLE", t, 96, 10, 1, ANIM_LOOP);
            return plat;
        }
        else if (type == "PLATFORM_2") {
            Actor* plat2 = actors.createPlatform(x, y, 80, 26, GREEN_E, "Platform2", 80, 26);
            platList.push_back(PlatformController(plat2, PING_PONG_X, 120, -2.0f));
            Texture2D t = spr(sprites, "PLAT_2");
            if (t.id) plat2->addAnimation("IDLE", t, 80, 10, 1, ANIM_LOOP);
            return plat2;
        }

        // ============================================================
        // BLOQUES
        // ============================================================
        else if (type == "MUSHROOM_BLOCK" || type == "BRICK_BLOCK" ||
            type == "COIN_BLOCK_SINGLE" || type == "COIN_BLOCK_MULTI" ||
            type == "FLOWER_BLOCK" || type == "1UP_BLOCK" ||
            type == "STAR_BLOCK")
        {
            Actor* block = actors.createObstacle(x, y, 32, 32, PINK_E, "Block", 32, 32, false, -1);
            block->affectedByGravity = false;
            block->isSolid = true;
            block->isHitableFromBelow = true;

            // Cerebro y contenido 
            if (type == "MUSHROOM_BLOCK") {
                aiList.push_back(AIController(block, BLOCK_BEH_MUSHROOM));
                Actor* mushroom = ActorFactory::spawn("ITEM_MUSHROOM", x, y, actors, aiList, platList, sprites);
                if (mushroom) {
                    mushroom->isHidden = true; mushroom->isStored = true; mushroom->isAwake = false;
                    mushroom->affectedByGravity = false;
                    aiList.back().behavior = MUSHROOM_EMERGING;
                    block->storedItem = mushroom;
                }
            }
            else if (type == "FLOWER_BLOCK") {
                aiList.push_back(AIController(block, BLOCK_BEH_FLOWER));
                Actor* flower = ActorFactory::spawn("ITEM_FLOWER", x, y, actors, aiList, platList, sprites);
                if (flower) {
                    flower->isHidden = true; flower->isStored = true; flower->isAwake = false;
                    flower->affectedByGravity = false;
                    block->storedItem = flower;
                }
            }
            else if (type == "1UP_BLOCK") {
                aiList.push_back(AIController(block, BLOCK_BEH_MUSHROOM));
                Actor* oneUp = ActorFactory::spawn("ITEM_1UP", x, y, actors, aiList, platList, sprites);
                if (oneUp) {
                    oneUp->isHidden = true; oneUp->isStored = true; oneUp->isAwake = false;
                    oneUp->affectedByGravity = false;
                    aiList.back().behavior = MUSHROOM_EMERGING;
                    block->storedItem = oneUp;
                }
            }
            else if (type == "STAR_BLOCK") {
                aiList.push_back(AIController(block, BLOCK_BEH_MUSHROOM));
                Actor* star = ActorFactory::spawn("ITEM_STAR", x, y, actors, aiList, platList, sprites);
                if (star) {
                    star->isHidden = true; star->isStored = true; star->isAwake = false;
                    star->affectedByGravity = false;
                    aiList.back().behavior = MUSHROOM_EMERGING;
                    block->storedItem = star;
                }
            }
            else if (type == "BRICK_BLOCK") {
                aiList.push_back(AIController(block, BLOCK_BEH_BRICKS));
            }
            else if (type == "COIN_BLOCK_SINGLE" || type == "COIN_BLOCK_MULTI") {
                EnemyBehavior coinBeh = (type == "COIN_BLOCK_SINGLE") ? BLOCK_BEH_COIN : BLOCK_BEH_MULTICOIN;
                aiList.push_back(AIController(block, coinBeh));

                Actor* blockCoin = actors.createItem(x, y, 32, 32, RED_E, "BlockCoin", 32, 32);
                blockCoin->powerUpType = 0;   // fantasma visual, no coleccionable
                blockCoin->isHidden = true;
                blockCoin->isStored = true;
                blockCoin->isAwake = false;
                blockCoin->affectedByGravity = false;
                aiList.push_back(AIController(blockCoin, COIN_EMERGING));
                Texture2D tc = spr(sprites, "COIN_SPRITE");
                if (tc.id) blockCoin->addAnimation("IDLE", tc, 32, 4, 4, ANIM_LOOP);
                block->storedItem = blockCoin;
            }

            // Animaciones del bloque
            { Texture2D t = spr(sprites, "MUSHBLOCK_NOTHIT");  if (t.id) block->addAnimation("MUSH_NOTHIT", t, 32, 10, 1, ANIM_ONCE); }
            { Texture2D t = spr(sprites, "MUSHBLOCK_HIT");     if (t.id) block->addAnimation("MUSH_HIT", t, 32, 10, 1, ANIM_ONCE); }
            { Texture2D t = spr(sprites, "BRICKBLOCK_NOTHIT"); if (t.id) block->addAnimation("BRICK_NOTHIT", t, 32, 10, 1, ANIM_ONCE); }

            // Muerte del ladrillo
            if (type == "BRICK_BLOCK") {
                Texture2D t = spr(sprites, "BRICK_SHATTER");
                if (t.id) {
                    block->deathTexture = t;
                    block->defaultDeathMovement = VFX_UP_AND_DOWN;
                    block->deathFrames = 6;
                    block->deathTimeInterval = 4;
                    block->deathW = 400;
                    block->deathH = 400;
                }
            }
            return block;
        }

        // ============================================================
        // ITEMS
        // ============================================================
        else if (type == "ITEM_COIN") {
            Actor* coin = actors.createItem(x, y, 30, 30, RED_E, "MapCoin", 32, 32);
            coin->powerUpType = 3;
            coin->affectedByGravity = false;
            aiList.push_back(AIController(coin, COIN_IDLE));
            Texture2D t = spr(sprites, "COIN_SPRITE");
            if (t.id) coin->addAnimation("IDLE", t, 32, 5, 4, ANIM_LOOP);
            return coin;
        }
        else if (type == "ITEM_MUSHROOM") {
            Actor* mush = actors.createItem(x, y, 30, 30, RED_E, "PowerUp", 32, 32);
            mush->powerUpType = 1;
            aiList.push_back(AIController(mush, MUSHROOM_WALK));
            // Ambas animaciones cargadas: el bloque elige en runtime cual activar
            { Texture2D t = spr(sprites, "MUSHROOM_SPRITE"); if (t.id) mush->addAnimation("IDLE", t, 32, 10, 1, ANIM_ONCE); }
            { Texture2D t = spr(sprites, "FLOWER_SPRITE");   if (t.id) mush->addAnimation("FLOWER_IDLE", t, 32, 5, 4, ANIM_LOOP); }
            return mush;
        }
        else if (type == "ITEM_FLOWER") {
            Actor* flower = actors.createItem(x, y, 30, 30, RED_E, "Flower", 32, 32);
            flower->powerUpType = 2;
            aiList.push_back(AIController(flower, FLOWER_EMERGING));
            Texture2D t = spr(sprites, "FLOWER_SPRITE");
            if (t.id) flower->addAnimation("IDLE", t, 32, 5, 4, ANIM_LOOP);
            return flower;
        }
        else if (type == "ITEM_1UP") {
            Actor* oneUp = actors.createItem(x, y, 30, 30, GREEN_E, "1Up", 32, 32);
            oneUp->powerUpType = 4;
            aiList.push_back(AIController(oneUp, MUSHROOM_WALK));
            Texture2D t = spr(sprites, "1UP_SPRITE");
            if (t.id) oneUp->addAnimation("IDLE", t, 32, 10, 1, ANIM_ONCE);
            return oneUp;
        }
        else if (type == "ITEM_STAR") {
            Actor* star = actors.createItem(x, y, 30, 30, GREEN_E, "Star", 32, 32);
            star->powerUpType = 5;
            aiList.push_back(AIController(star, MUSHROOM_WALK));
            Texture2D t = spr(sprites, "STAR_SPRITE");
            if (t.id) star->addAnimation("IDLE", t, 32, 5, 4, ANIM_LOOP);
            return star;
        }

        // ============================================================
        // ENEMIGOS
        // ============================================================
        else if (type == "ENEMY_GOOPY") {
            Actor* goopy = actors.createEnemy(x, y, 60, 80, GREEN_E, "Goopy", 100, 92);
            goopy->lookingLeft = true;
            aiList.push_back(AIController(goopy, PATROL_SHOOTER));
            { Texture2D t = spr(sprites, "GOOPY_IDLE");  if (t.id) goopy->addAnimation("IDLE", t, 100, 10, 1, ANIM_LOOP); }
            { Texture2D t = spr(sprites, "GOOPY_SHOOT"); if (t.id) goopy->addAnimation("SHOOT", t, 100, 5, 1, ANIM_ONCE); }
            return goopy;
        }
        else if (type == "ENEMY_JUMPING_GOOPY") {
            Actor* goopy = actors.createEnemy(x, y, 60, 80, GREEN_E, "Goopy", 100, 92);
            goopy->lookingLeft = true;
            aiList.push_back(AIController(goopy, JUMP_MEDUSA));
            { Texture2D t = spr(sprites, "GOOPY_IDLE");  if (t.id) goopy->addAnimation("IDLE", t, 100, 10, 1, ANIM_LOOP); }
            { Texture2D t = spr(sprites, "GOOPY_SHOOT"); if (t.id) goopy->addAnimation("SHOOT", t, 100, 5, 1, ANIM_ONCE); }
            return goopy;
        }
        else if (type == "ENEMY_GOOMBA") {
            Actor* goomba = actors.createEnemy(x, y, 50, 50, RED_E, "Goomba", 50, 50);
            goomba->lookingLeft = true;
            goomba->vidas = 1;
            goomba->isStompable = true;
            goomba->immuneToBullets = true;
            aiList.push_back(AIController(goomba, PATROL_SHOOTER));
            Texture2D t = spr(sprites, "GOOMBA_WALK");
            if (t.id) goomba->addAnimation("IDLE", t, 50, 10, 1, ANIM_LOOP);
            return goomba;
        }
        else if (type == "ENEMY_GOOMBA_SMALL") {
            Actor* gs = actors.createEnemy(x, y, 30, 30, RED_E, "GoombaS", 50, 50);
            gs->lookingLeft = true;
            gs->vidas = 1;
            gs->isStompable = true;
            gs->immuneToBullets = false;
            aiList.push_back(AIController(gs, GOOMBA_INDEFENSO));
            { Texture2D t = spr(sprites, "GOOMBAS_WALK");    if (t.id) gs->addAnimation("IDLE", t, 50, 10, 2, ANIM_LOOP); }
            // Muerte aplastado (DEATH_SQUASHED)
            {
                Texture2D t = spr(sprites, "GOOMBA_SQUASHED");
                if (t.id) {
                    gs->deathTexture = t;
                    gs->defaultDeathMovement = VFX_STATIC;
                    gs->deathFrames = 1;
                    gs->deathTimeInterval = 60;
                    gs->deathW = 50;
                    gs->deathH = 50;
                }
            }
            
            // Muerte volteado (DEATH_FLIPPED) → misma textura, ActorManager activa VFX_PARABOLA
            { Texture2D t = spr(sprites, "GOOMBA_FLIPPED");    if (t.id) gs->deathSpriteFlipped = t; }
            return gs;
        }
        else if (type == "GREEN_KOOPA") {
            Actor* koopa = actors.createEnemy(x, y, 30, 48, RED_E, "KoopaG", 100, 100);
            koopa->lookingLeft = true;
            koopa->vidas = 2;
            koopa->isStompable = true;
            koopa->immuneToBullets = false;
            aiList.push_back(AIController(koopa, GREEN_KOOPA_WALK));
            { Texture2D t = spr(sprites, "GREEN_KOOPA_WALK");  if (t.id) koopa->addAnimation("WALK", t, 100, 10, 2, ANIM_LOOP); }
            { Texture2D t = spr(sprites, "GREEN_SHELL_IDLE");  if (t.id) koopa->addAnimation("SHELL_IDLE", t, 100, 10, 1, ANIM_ONCE); }
            { Texture2D t = spr(sprites, "GREEN_SHELL_SLIDE"); if (t.id) koopa->addAnimation("SHELL_SLIDE", t, 100, 5, 1, ANIM_ONCE); }
            {
                Texture2D t = spr(sprites, "GREEN_SHELL_FLIPPED");
                if (t.id) {
                    koopa->deathSpriteFlipped = t;
                    koopa->deathTexture = t;
                    koopa->defaultDeathMovement = VFX_PARABOLA;
                    koopa->deathFrames = 1;
                    koopa->deathTimeInterval = 60;
                    koopa->deathW = 100;
                    koopa->deathH = 100;
                }
            }
            return koopa;
        }
        else if (type == "RED_KOOPA") {
            Actor* koopa = actors.createEnemy(x, y, 30, 48, RED_E, "KoopaR", 100, 100);
            koopa->lookingLeft = true;
            koopa->vidas = 2;
            koopa->isStompable = true;
            koopa->immuneToBullets = false;
            aiList.push_back(AIController(koopa, RED_KOOPA_WALK));
            { Texture2D t = spr(sprites, "RED_KOOPA_WALK");  if (t.id) koopa->addAnimation("WALK", t, 100, 10, 2, ANIM_LOOP); }
            { Texture2D t = spr(sprites, "RED_SHELL_IDLE");  if (t.id) koopa->addAnimation("SHELL_IDLE", t, 100, 10, 1, ANIM_ONCE); }
            { Texture2D t = spr(sprites, "RED_SHELL_SLIDE"); if (t.id) koopa->addAnimation("SHELL_SLIDE", t, 100, 5, 1, ANIM_ONCE); }
            {
                Texture2D t = spr(sprites, "RED_SHELL_FLIPPED");
                if (t.id) {
                    koopa->deathSpriteFlipped = t;
                    koopa->deathTexture = t;
                    koopa->defaultDeathMovement = VFX_PARABOLA;
                    koopa->deathFrames = 1;
                    koopa->deathTimeInterval = 60;
                    koopa->deathW = 100;
                    koopa->deathH = 100;
                }
            }
            return koopa;
        }

        // ============================================================
        // JUGADORES
        // ============================================================
        else if (type == "PLAYER1") {
            return actors.createPlayer(x, y, 30, 40, WHITE_E, "Player1", 100, 100);
        }
        else if (type == "PLAYER2") {
            Actor* p2 = actors.createPlayer(x, y, 40, 40, WHITE_E, "Player2", 100, 100);
            p2->lookingLeft = true;
            return p2;
        }
        else if (type == "PLAYER_MARIO") {
            Actor* p1 = actors.createPlayer(x, y, 25, 25, WHITE_E, "Mario", 100, 100);
            p1->useInertia = true;
            p1->maxSpeed = 3.0f;
            p1->turboIncrFactor = 2.0f;
            p1->acceleration = 0.2f;
            p1->deceleration = 0.3f;
            { Texture2D t = spr(sprites, "MARIO_SMALL_IDLE"); if (t.id) p1->addAnimation("SMALL_IDLE", t, 100, 10, 1, ANIM_ONCE); }
            { Texture2D t = spr(sprites, "MARIO_SMALL_RUN");  if (t.id) p1->addAnimation("SMALL_RUN", t, 100, 5, 3, ANIM_LOOP); }
            { Texture2D t = spr(sprites, "MARIO_BIG_IDLE");   if (t.id) p1->addAnimation("BIG_IDLE", t, 100, 10, 1, ANIM_ONCE); }
            { Texture2D t = spr(sprites, "MARIO_BIG_RUN");    if (t.id) p1->addAnimation("BIG_RUN", t, 100, 5, 3, ANIM_LOOP); }
            { Texture2D t = spr(sprites, "MARIO_DEATH");      if (t.id) p1->addAnimation("DEATH", t, 100, 10, 1, ANIM_ONCE); }
            return p1;
        }

        // ============================================================
        // OBSTACULOS
        // ============================================================
        else if (type == "OBSTACLE_BLOCK") {
            return actors.createObstacle(x, y, 64, 104, RED_E, "Block", 64, 104, true);
        }
        else if (type == "OBSTACLE_BOX") {
            return actors.createObstacle(x, y, 48, 48, PINK_E, "Box", 48, 48, true, 5);
        }
        else if (type == "OBSTACLE_WALL") {
            return actors.createObstacle(x, y, 32, 160, BLUE_E, "Wall", 32, 160, true);
        }
        else if (type == "YAMILE") {
            Actor* goal = actors.createObstacle(x, y, 100, 100, BLUE_E, "Goal", 100, 100, false, -1);
            goal->isSolid = false;
            goal->affectedByGravity = false;
            Texture2D t = spr(sprites, "YAMILE_SPRITE");
            if (t.id) goal->addAnimation("IDLE", t, 100, 30, 2, ANIM_LOOP);
            return goal;
        }
        else if (type == "GOAL_POINT") {
            Actor* goal = actors.createObstacle(x, y, 64, 334, BLUE_E, "Goal", 64, 334, false, -1);
            goal->isSolid = false;
            goal->affectedByGravity = false;
            Texture2D t = spr(sprites, "GOAL_SPRITE");
            if (t.id) goal->addAnimation("IDLE", t, 64, 30, 2, ANIM_LOOP);
            return goal;
        }
        else if (type == "PIPE") {
            Actor* pipe = actors.createObstacle(x, y, 64, 255, PINK_E, "pipe", 64, 256, false, -1);
            pipe->isSolid = true;
            pipe->affectedByGravity = false;
            aiList.push_back(AIController(pipe, STATIC_PIPE));
            Texture2D t = spr(sprites, "PIPE_DEFAULT");
            if (t.id) pipe->addAnimation("PIPE_DEFAULT", t, 64, 10, 1, ANIM_ONCE);
            return pipe;
        }

        // ============================================================
        // PROYECTILES
        // ============================================================
        else if (type == "PROJECTILE_FIREBALL") {
            Actor* fb = actors.createPlayerFireball(x, y, 16, 16, RED_E, "Fireball", 16, 16);
            // fb->setSphereCollider(7.0f, 8.0f, 8.0f); <-- Sphere collider hace que muera al tocar el suelo, revisar
            aiList.push_back(AIController(fb, FIREBALL_BEH));
            { Texture2D t = spr(sprites, "FIREBALL_SPRITE");    if (t.id) fb->addAnimation("IDLE", t, 16, 4, 4, ANIM_LOOP); }
            {
                Texture2D t = spr(sprites, "FIREBALL_EXPLOSION");
                if (t.id) {
                    fb->deathTexture = t;
                    fb->defaultDeathMovement = VFX_STATIC;
                    fb->deathFrames = 3;
                    fb->deathTimeInterval = 4;
                    fb->deathW = 16;
                    fb->deathH = 16;
                }
            }
            return fb;
        }

        return NULL;
    }

private:
    // Helper para buscar una textura sin buscar su clave en el map
    static Texture2D spr(std::map<std::string, Texture2D>& sprites,
        const std::string& key)
    {
        std::map<std::string, Texture2D>::iterator it = sprites.find(key);
        if (it != sprites.end() && it->second.id != 0)
            return it->second;
        Texture2D empty = { 0 };
        return empty;
    }
};

#endif // ACTORFACTORY_H