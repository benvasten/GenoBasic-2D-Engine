#ifndef ACTORMANAGER_H
#define ACTORMANAGER_H

// ============================================================
// ACTORMANAGER.H
// ============================================================

#include "raylib.h"
#include <vector>
#include "Actor.h"
#include "Bullet.h"
#include "color.h"

#include "Camera.h"          // GameCamera
#include "CollisionActors.h"
#include "TileMap.h"
#include "VFXManager.h"


class ActorManager {
private:
    std::vector<Actor*>  players;
    std::vector<Actor*>  enemies;
    std::vector<Actor*>  obstacles;
    std::vector<Actor*>  platforms;
    std::vector<Actor*>  playerFireballs;
    std::vector<Actor*>  items;
    std::vector<Bullet*> playerBullets;
    std::vector<Bullet*> enemyBullets;

public:
    ActorManager() {}
    ~ActorManager() { clearAll(); }

    // ============================================================
    // METODOS DE FABRICA
    // ============================================================

    Actor* createPlayer(float x, float y, int w, int h,
        EngineColor color, const char* name, int visW, int visH)
    {
        Actor* a = new Actor(x, y, w, h, color, name, visW, visH,
            true,   // affectedByGravity
            false,  // canCarry
            true,   // canPush
            true);  // isPushable
        players.push_back(a);
        return a;
    }

    Actor* createEnemy(float x, float y, int w, int h,
        EngineColor color, const char* name, int visW, int visH)
    {
        Actor* a = new Actor(x, y, w, h, color, name, visW, visH,
            false,  // gravity
            false,  // canCarry
            true,   // canPush
            true);  // isPushable
        a->isSolid = false;
        a->isAwake = false;   // Todos los enemigos nacen dormidos
        enemies.push_back(a);
        return a;
    }

    Actor* createItem(float x, float y, int w, int h,
        EngineColor color, const char* name, int visW, int visH)
    {
        Actor* a = new Actor(x, y, w, h, color, name, visW, visH,
            true, false, false, false);
        a->isSolid = false;
        a->isAwake = false;
        items.push_back(a);
        return a;
    }

    Actor* createObstacle(float x, float y, int w, int h,
        EngineColor color, const char* name, int visW, int visH,
        bool pushable = true, int hp = -1)
    {
        Actor* a = new Actor(x, y, w, h, color, name, visW, visH,
            true,      // gravity
            false,     // canCarry
            false,     // canPush
            pushable);
        a->vidas = hp;   // -1 = invencible
        obstacles.push_back(a);
        return a;
    }

    Actor* createPlatform(float x, float y, int w, int h,
        EngineColor color, const char* name, int visW, int visH)
    {
        Actor* a = new Actor(x, y, w, h, color, name, visW, visH,
            false,  // gravity
            true,   // canCarry
            true,   // canPush
            true);  // isPushable
        platforms.push_back(a);
        return a;
    }

    Actor* createPlayerFireball(float x, float y, int w, int h,
        EngineColor color, const char* name, int visW, int visH)
    {
        Actor* a = new Actor(x, y, w, h, color, name, visW, visH,
            true, false, false, false);
        a->isSolid = false;
        playerFireballs.push_back(a);
        return a;
    }

    // ============================================================
    // BULLETS
    // ============================================================

    void addPlayerBullet(Bullet* bullet) { if (bullet) playerBullets.push_back(bullet); }
    void addEnemyBullet(Bullet* bullet) { if (bullet) enemyBullets.push_back(bullet); }

    // ============================================================
    // GETTERS
    // ============================================================

    std::vector<Actor*>& getPlayers() { return players; }
    std::vector<Actor*>& getEnemies() { return enemies; }
    std::vector<Actor*>& getObstacles() { return obstacles; }
    std::vector<Actor*>& getPlatforms() { return platforms; }
    std::vector<Actor*>& getItems() { return items; }
    std::vector<Actor*>& getPlayerFireballs() { return playerFireballs; }
    std::vector<Bullet*>& getPlayerBullets() { return playerBullets; }
    std::vector<Bullet*>& getEnemyBullets() { return enemyBullets; }

    const std::vector<Actor*>& getPlayers()         const { return players; }
    const std::vector<Actor*>& getEnemies()         const { return enemies; }
    const std::vector<Actor*>& getObstacles()       const { return obstacles; }
    const std::vector<Actor*>& getPlatforms()       const { return platforms; }
    const std::vector<Actor*>& getItems()           const { return items; }
    const std::vector<Actor*>& getPlayerFireballs() const { return playerFireballs; }

    // ============================================================
    // COMBINACIONES
    // ============================================================

    std::vector<Actor*> getAllActors() const {
        std::vector<Actor*> all;
        all.reserve(players.size() + enemies.size() + obstacles.size() +
            platforms.size() + items.size() + playerFireballs.size());
        for (size_t i = 0; i < players.size(); i++) all.push_back(players[i]);
        for (size_t i = 0; i < enemies.size(); i++) all.push_back(enemies[i]);
        for (size_t i = 0; i < obstacles.size(); i++) all.push_back(obstacles[i]);
        for (size_t i = 0; i < platforms.size(); i++) all.push_back(platforms[i]);
        for (size_t i = 0; i < items.size(); i++) all.push_back(items[i]);
        for (size_t i = 0; i < playerFireballs.size(); i++) all.push_back(playerFireballs[i]);
        return all;
    }

    std::vector<Actor*> getCarriers() const {
        std::vector<Actor*> carriers;
        for (size_t i = 0; i < platforms.size(); i++) carriers.push_back(platforms[i]);
        for (size_t i = 0; i < enemies.size(); i++)
            if (enemies[i]->canCarry) carriers.push_back(enemies[i]);
        return carriers;
    }

    std::vector<Actor*> getNonCarriers() const {
        std::vector<Actor*> nc;
        for (size_t i = 0; i < players.size(); i++) nc.push_back(players[i]);
        for (size_t i = 0; i < obstacles.size(); i++) nc.push_back(obstacles[i]);
        for (size_t i = 0; i < items.size(); i++) nc.push_back(items[i]);
        for (size_t i = 0; i < playerFireballs.size(); i++) nc.push_back(playerFireballs[i]);
        for (size_t i = 0; i < enemies.size(); i++)
            if (!enemies[i]->canCarry) nc.push_back(enemies[i]);
        return nc;
    }

    // ============================================================
    // ACTUALIZACIÓN DE BULLETS
    // ============================================================

    void updateBullets() {
        for (size_t i = 0; i < playerBullets.size(); i++) playerBullets[i]->update();
        for (size_t i = 0; i < enemyBullets.size(); i++) enemyBullets[i]->update();
    }

    void removeOffscreenBullets(int mapWidthPixels, int mapHeightPixels) {
        for (std::vector<Bullet*>::iterator it = playerBullets.begin(); it != playerBullets.end(); ) {
            if ((*it)->isOffScreen(mapWidthPixels, mapHeightPixels)) { delete* it; it = playerBullets.erase(it); }
            else ++it;
        }
        for (std::vector<Bullet*>::iterator it = enemyBullets.begin(); it != enemyBullets.end(); ) {
            if ((*it)->isOffScreen(mapWidthPixels, mapHeightPixels)) { delete* it; it = enemyBullets.erase(it); }
            else ++it;
        }
    }

    // ============================================================
    // COLISIONES BULLETS
    // ============================================================

    void checkBulletTilemapCollisions(const TileMap& tilemap) {
        for (std::vector<Bullet*>::iterator it = playerBullets.begin(); it != playerBullets.end(); ) {
            Bullet* b = *it;
            int l = (int)(b->collider.x) / TILE_SIZE;
            int r = (int)(b->collider.x + b->collider.w) / TILE_SIZE;
            int t = (int)(b->collider.y) / TILE_SIZE;
            int bot = (int)(b->collider.y + b->collider.h) / TILE_SIZE;
            if (tilemap.isSolid(l, t) || tilemap.isSolid(r, t) ||
                tilemap.isSolid(l, bot) || tilemap.isSolid(r, bot))
            {
                delete b; it = playerBullets.erase(it);
            }
            else ++it;
        }
        for (std::vector<Bullet*>::iterator it = enemyBullets.begin(); it != enemyBullets.end(); ) {
            Bullet* b = *it;
            int l = (int)(b->collider.x) / TILE_SIZE;
            int r = (int)(b->collider.x + b->collider.w) / TILE_SIZE;
            int t = (int)(b->collider.y) / TILE_SIZE;
            int bot = (int)(b->collider.y + b->collider.h) / TILE_SIZE;
            if (tilemap.isSolid(l, t) || tilemap.isSolid(r, t) ||
                tilemap.isSolid(l, bot) || tilemap.isSolid(r, bot))
            {
                delete b; it = enemyBullets.erase(it);
            }
            else ++it;
        }
    }

    // hitSound  → sonado cuando el obstáculo aguanta el impacto
    // breakSound → sonado cuando el obstáculo se destruye
    void checkBulletObstacleCollisions(Sound hitSound, Sound breakSound) {
        for (std::vector<Bullet*>::iterator it = playerBullets.begin(); it != playerBullets.end(); ) {
            bool destroyed = false;
            Bullet* bullet = *it;
            for (size_t o = 0; o < obstacles.size(); o++) {
                Actor* obs = obstacles[o];
                if (!obs->alive || !obs->isSolid) continue;
                if (!checkAABB(bullet->collider.x, bullet->collider.y, bullet->collider.w, bullet->collider.h,
                    obs->collider.x, obs->collider.y, obs->collider.w, obs->collider.h)) continue;

                if (obs->vidas > 0) {
                    obs->vidas--;
                    if (obs->vidas <= 0) { obs->alive = false; PlaySound(breakSound); }
                    else { PlaySound(hitSound); }
                }
                // La bala siempre se destruye (incluso contra obstáculos invencibles)
                delete bullet; it = playerBullets.erase(it);
                destroyed = true;
                break;
            }
            if (!destroyed) ++it;
        }

        for (std::vector<Bullet*>::iterator it = enemyBullets.begin(); it != enemyBullets.end(); ) {
            bool destroyed = false;
            Bullet* bullet = *it;
            for (size_t o = 0; o < obstacles.size(); o++) {
                Actor* obs = obstacles[o];
                if (!obs->alive || !obs->isSolid) continue;
                if (!checkAABB(bullet->collider.x, bullet->collider.y, bullet->collider.w, bullet->collider.h,
                    obs->collider.x, obs->collider.y, obs->collider.w, obs->collider.h)) continue;

                if (obs->vidas > 0) {
                    obs->vidas--;
                    if (obs->vidas <= 0) { obs->alive = false; PlaySound(breakSound); }
                    else { PlaySound(hitSound); }
                }
                delete bullet; it = enemyBullets.erase(it);
                destroyed = true;
                break;
            }
            if (!destroyed) ++it;
        }
    }

    // AQUÍ LOS SONIDOS ESTAN HARDCODEADOS (POR AHORA):
    // hitSound   → enemigo/jugador herido pero vivo
    // koSound    → enemigo/jugador eliminado
    // clankSound → bala rebotó contra inmune
    void checkBulletCollisions(Sound hitSound, Sound koSound, Sound clankSound) {
        // Player bullets vs enemies
        for (std::vector<Bullet*>::iterator it = playerBullets.begin(); it != playerBullets.end(); ) {
            bool destroyed = false;
            Bullet* bullet = *it;
            for (size_t e = 0; e < enemies.size(); e++) {
                Actor* enemy = enemies[e];
                if (!enemy->alive) continue;
                if (!checkAABB(bullet->collider.x, bullet->collider.y, bullet->collider.w, bullet->collider.h,
                    enemy->collider.x, enemy->collider.y, enemy->collider.w, enemy->collider.h)) continue;

                if (enemy->immuneToBullets) {
                    PlaySound(clankSound);
                }
                else {
                    enemy->vidas--;
                    if (enemy->vidas <= 0) { enemy->alive = false; PlaySound(koSound); }
                    else { PlaySound(hitSound); }
                }
                delete bullet; it = playerBullets.erase(it);
                destroyed = true;
                break;
            }
            if (!destroyed) ++it;
        }

        // Enemy bullets vs players
        for (std::vector<Bullet*>::iterator it = enemyBullets.begin(); it != enemyBullets.end(); ) {
            bool destroyed = false;
            Bullet* bullet = *it;
            for (size_t p = 0; p < players.size(); p++) {
                Actor* player = players[p];
                if (!player->alive || player->isInvincible) continue;
                if (!checkAABB(bullet->collider.x, bullet->collider.y, bullet->collider.w, bullet->collider.h,
                    player->collider.x, player->collider.y, player->collider.w, player->collider.h)) continue;

                player->vidas--;
                if (player->vidas <= 0) {
                    player->alive = false;
                    player->vx = 0; player->vy = 0;
                    PlaySound(koSound);
                }
                else {
                    PlaySound(hitSound);
                    player->triggerInvincibility();
                    player->vx = (player->collider.x < bullet->collider.x) ? -12.0f : 12.0f;
                    player->vy = -3.0f;
                }
                delete bullet; it = enemyBullets.erase(it);
                destroyed = true;
                break;
            }
            if (!destroyed) ++it;
        }
    }

    // ============================================================
    // COLISIONES ACTORES
    // ============================================================

    void checkPlayerEnemyCollisions(Sound hitSound, Sound koSound, Sound shrinkSound) {
        for (size_t p = 0; p < players.size(); p++) {
            Actor* player = players[p];
            for (size_t e = 0; e < enemies.size(); e++) {
                Actor* enemy = enemies[e];
                if (!player->alive || !enemy->alive) continue;
                if (!checkActorOverlap(player, enemy)) continue;

                // 0. Estrella: mata instantáneamente
                if (player->isStarInvincible) {
                    enemy->causeOfDeath = DEATH_FLIPPED;
                    enemy->vidas = 0;
                    enemy->alive = false;
                    PlaySound(koSound);
                    continue;
                }

                // 1. Stomp: jugador cae sobre la cabeza del enemigo
                if (enemy->isStompable && !player->isInvincible && player->vy > 0 &&
                    (player->prevY + player->collider.h) < (enemy->collider.y + enemy->collider.h * 0.8f))
                {
                    //player->isInvincible = true;
                    player->postStompGracePeriod = 8;
                    enemy->vidas--;
                    enemy->causeOfDeath = DEATH_SQUASHED;
                    if (enemy->vidas <= 0 && !enemy->isShell) enemy->alive = false;

                    if (enemy->isShell && enemy->vidas <= 0) enemy->kickGracePeriod = 10;

                    PlaySound(koSound);
                    player->vy = -player->jumpForce * 0.9f;
                    player->isGrounded = false;
                    player->hasDashedInAir = false;
                    continue;
                }

                // 2. Daño por contacto
                if (!player->isInvincible && player->postStompGracePeriod == 0) {


                    // Caparazón con período de gracia activo: no daña todavía
                    if (enemy->isShell && enemy->kickGracePeriod > 0) {
                        enemy->kickGracePeriod--;
                    }
                    // Mario pateó lateralmente el caparazón quieto
                    else if (!enemy->dealsDamage) {
                        enemy->vx = (player->collider.x < enemy->collider.x) ? 6.0f : -6.0f;
                        player->x += (player->collider.x < enemy->collider.x) ? -2.0f : 2.0f;
                        player->syncCollider();

                        enemy->dealsDamage = true;
                        enemy->kickGracePeriod = 10;  // mas o menos 10 frames para que se separen los colliders
                        PlaySound(koSound);
                    }
                    // Daño real por contacto con enemigo que ya hace daño
                    else {
                        player->vidas--;
                        if (player->vidas <= 0) {
                            player->isDying = true;
                            player->hitstopTimer = 180;
                            player->vx = 0; player->vy = 0;
                            PlaySound(koSound);
                        }
                        else {
                            player->triggerInvincibility();
                            player->isTransforming = true;
                            player->hitstopTimer = 60;
                            player->oldAnimPrefix = player->animPrefix;
                            player->targetAnimPrefix = "SMALL_";
                            player->vx = 0; player->vy = 0;
                            PlaySound(shrinkSound);
                            player->vx = (player->collider.x < enemy->collider.x)
                                ? (player->useInertia ? -2.0f : -12.0f)
                                : (player->useInertia ? 2.0f : 12.0f);
                            player->vy = -3.0f;
                        }
                    }
                }
            }
        }
    }

    void checkPlayerItemCollisions(Sound powerUpSound, Sound lupSound, Sound coinSound) {
        for (size_t p = 0; p < players.size(); p++) {
            Actor* player = players[p];
            if (!player->alive) continue;
            for (size_t i = 0; i < items.size(); i++) {
                Actor* item = items[i];
                if (!item->alive || item->powerUpType == 0) continue;
                if (!checkActorOverlap(player, item)) continue;

                item->alive = false;

                if (item->powerUpType == 1) {           // Champiñón
                    if (player->animPrefix == "SMALL_") {
                        player->isTransforming = true;
                        player->hitstopTimer = 60;
                        player->oldAnimPrefix = "SMALL_";
                        player->targetAnimPrefix = "BIG_";
                        player->vidas++;
                        player->vx = 0; player->vy = 0;
                    }
                    PlaySound(powerUpSound);
                }
                else if (item->powerUpType == 2) {      // Flor de fuego
                    if (player->animPrefix == "SMALL_" || player->animPrefix == "BIG_") {
                        player->isTransforming = true;
                        player->hitstopTimer = 60;
                        player->oldAnimPrefix = player->animPrefix;
                        player->targetAnimPrefix = "FIRE_";
                        player->vx = 0; player->vy = 0;
                    }
                    PlaySound(powerUpSound);
                }
                else if (item->powerUpType == 3) {      // Moneda
                    PlaySound(coinSound);
                }
                else if (item->powerUpType == 4) {      // 1-Up
                    PlaySound(lupSound);
                }
                else if (item->powerUpType == 5) {      // Estrella
                    player->isStarInvincible = true;
                    player->starTimer = 720;     // 10 segundos a 60 FPS
                }
            }
        }
    }

    // Bloque subiendo (isLavaFloored) golpea a un enemigo encima
    void checkObstacleEnemyCollisions(Sound koSound) {
        for (size_t p = 0; p < obstacles.size(); p++) {
            Actor* obstacle = obstacles[p];
            for (size_t e = 0; e < enemies.size(); e++) {
                Actor* enemy = enemies[e];
                if (!obstacle->alive || !enemy->alive) continue;
                if (!checkActorOverlap(obstacle, enemy)) continue;

                if (obstacle->isLavaFloored && !enemy->isInvincible &&
                    (enemy->collider.y + enemy->collider.h) <= (obstacle->collider.y + (obstacle->collider.h / 2)))
                {
                    enemy->causeOfDeath = DEATH_FLIPPED;
                    enemy->vidas--;
                    if (enemy->vidas <= 0) {
                        enemy->alive = false;
                        PlaySound(koSound);
                    }
                    else {
                        enemy->vy = -5.0f;
                        enemy->y -= 8.0f;
                        enemy->syncCollider();
                        enemy->isGrounded = false;
                        PlaySound(koSound);
                    }
                }
            }
        }
    }

    // Bloque subiendo lanza un ítem hacia arriba e invierte su dirección
    void checkObstacleItemCollisions() {
        for (size_t p = 0; p < obstacles.size(); p++) {
            Actor* obstacle = obstacles[p];
            for (size_t i = 0; i < items.size(); i++) {
                Actor* item = items[i];
                if (item->powerUpType == 0) continue;
                if (!obstacle->alive || !item->alive) continue;
                if (!checkActorOverlap(obstacle, item)) continue;

                if (obstacle->isLavaFloored &&
                    (item->collider.y + item->collider.h) <= (obstacle->collider.y + (obstacle->collider.h / 2)))
                {
                    item->vy = -6.0f;
                    item->isGrounded = false;
                    item->vx = 0;       // AIController lo interpreta como colisión con pared → invierte
                    item->y -= 4.0f;
                    item->syncCollider();
                }
            }
        }
    }

    void checkFireballEnemyCollisions(Sound koSound) {
        for (size_t f = 0; f < playerFireballs.size(); f++) {
            Actor* fb = playerFireballs[f];
            if (!fb->alive) continue;
            for (size_t e = 0; e < enemies.size(); e++) {
                Actor* enemy = enemies[e];
                if (!enemy->alive || enemy->isStored) continue;
                if (!checkActorOverlap(fb, enemy)) continue;

                fb->alive = false;
                if (!enemy->immuneToBullets) {
                    enemy->causeOfDeath = DEATH_FLIPPED;
                    enemy->vidas--;
                    if (enemy->vidas <= 0) { enemy->alive = false; PlaySound(koSound); }
                }
                break;
            }
        }
    }

    // Caparazón deslizándose elimina a otros enemigos
    void checkEnemyEnemyCollisions(Sound koSound) {
        for (size_t i = 0; i < enemies.size(); i++) {
            Actor* e1 = enemies[i];
            if (!e1->alive || !e1->isShell || !e1->dealsDamage || e1->vx == 0) continue;
            for (size_t j = 0; j < enemies.size(); j++) {
                if (i == j) continue;
                Actor* e2 = enemies[j];
                if (!e2->alive || e2->isStored) continue;
                if (!checkActorOverlap(e1, e2)) continue;

                e2->causeOfDeath = DEATH_FLIPPED;
                e2->vidas = 0;
                e2->alive = false;
                PlaySound(koSound);
            }
        }
    }

    // ============================================================
    // ZONA DE MUERTE
    // ============================================================

    void checkOutOfBounds(float minX, float maxX, float killY) {
        for (size_t i = 0; i < players.size(); i++) {
            Actor* p = players[i];
            if (p->alive && !p->isDying &&
                (p->y > killY || p->x < minX || p->x > maxX))
            {
                p->vidas = 0;
                p->alive = false;
            }
        }
        for (size_t i = 0; i < enemies.size(); i++) {
            Actor* e = enemies[i];
            if (e->alive && (e->y > killY || e->x < minX || e->x > maxX)) {
                e->vidas = 0;
                e->alive = false;
                e->deathTexture = { 0 };   // Sin animación de muerte al caer al vacío
            }
        }
        for (size_t i = 0; i < obstacles.size(); i++) {
            Actor* o = obstacles[i];
            if (o->alive && (o->y > killY || o->x < minX || o->x > maxX)) {
                o->vidas = 0;
                o->alive = false;
                o->deathTexture = { 0 };
            }
        }
        for (size_t i = 0; i < items.size(); i++) {
            Actor* it = items[i];
            if (it->alive && (it->y > killY || it->x < minX || it->x > maxX)) {
                it->vidas = 0;
                it->alive = false;
            }
        }
    }

    // ============================================================
    // LIMPIEZA
    // ============================================================

    void removeDeadActors(VFXManager& vfx) {
        removeDeadFromVector(enemies, vfx);
        removeDeadFromVector(obstacles, vfx);
        removeDeadFromVector(platforms, vfx);
        removeDeadFromVector(items, vfx);
        removeDeadFromVector(playerFireballs, vfx);
    }

    void clearAll() {
        clearVector(players);
        clearVector(enemies);
        clearVector(obstacles);
        clearVector(platforms);
        clearVector(items);
        clearVector(playerFireballs);
        clearBulletVector(playerBullets);
        clearBulletVector(enemyBullets);
    }

    // ============================================================
    // RENDERIZADO
    // ============================================================

    void drawAll(const GameCamera& cam) const {
        for (size_t i = 0; i < playerFireballs.size(); i++)
            if (!playerFireballs[i]->isHidden)
                playerFireballs[i]->draw((int)cam.x, (int)cam.y);

        for (size_t i = 0; i < items.size(); i++)
            if (!items[i]->isHidden)
                items[i]->draw((int)cam.x, (int)cam.y);

        for (size_t i = 0; i < playerBullets.size(); i++)
            playerBullets[i]->draw((int)cam.x, (int)cam.y);

        for (size_t i = 0; i < enemyBullets.size(); i++)
            enemyBullets[i]->draw((int)cam.x, (int)cam.y);

        for (size_t i = 0; i < obstacles.size(); i++)
            obstacles[i]->draw_old((int)cam.x, (int)cam.y);

        for (size_t i = 0; i < platforms.size(); i++) {
            // Offset visual de plataformas (igual que el original)
            int offsetY = 0;
            if (platforms[i]->name == "Platform")  offsetY = 13;
            else if (platforms[i]->name == "Platform2") offsetY = 6;
            platforms[i]->draw_old((int)cam.x, (int)cam.y + offsetY);
        }

        for (size_t i = 0; i < enemies.size(); i++)
            if (enemies[i]->alive)
                enemies[i]->draw((int)cam.x, (int)cam.y);

        for (size_t i = 0; i < players.size(); i++)
            if (players[i]->alive)
                players[i]->draw((int)cam.x, (int)cam.y);
    }

    // Debug: dibuja solo los colliders de todos los actores
    void drawAllColliders(const GameCamera& cam, Color debugColor) const {
        for (size_t i = 0; i < playerFireballs.size(); i++)
            playerFireballs[i]->debugDrawCollider((int)cam.x, (int)cam.y, debugColor);
        for (size_t i = 0; i < playerBullets.size(); i++) {
            DrawRectangleLines(
                (int)playerBullets[i]->collider.x - (int)cam.x,
                (int)playerBullets[i]->collider.y - (int)cam.y,
                playerBullets[i]->collider.w,
                playerBullets[i]->collider.h,
                debugColor);
        }
        for (size_t i = 0; i < enemyBullets.size(); i++) {
            DrawRectangleLines(
                (int)enemyBullets[i]->collider.x - (int)cam.x,
                (int)enemyBullets[i]->collider.y - (int)cam.y,
                enemyBullets[i]->collider.w,
                enemyBullets[i]->collider.h,
                debugColor);
        }
        for (size_t i = 0; i < items.size(); i++)
            if (!items[i]->isHidden)
                items[i]->debugDrawCollider((int)cam.x, (int)cam.y, debugColor);
        for (size_t i = 0; i < obstacles.size(); i++)
            obstacles[i]->debugDrawCollider((int)cam.x, (int)cam.y, debugColor);
        for (size_t i = 0; i < platforms.size(); i++)
            platforms[i]->debugDrawCollider((int)cam.x, (int)cam.y, debugColor);
        for (size_t i = 0; i < enemies.size(); i++)
            if (enemies[i]->alive)
                enemies[i]->debugDrawCollider((int)cam.x, (int)cam.y, debugColor);
        for (size_t i = 0; i < players.size(); i++)
            if (players[i]->alive)
                players[i]->debugDrawCollider((int)cam.x, (int)cam.y, debugColor);
    }

private:
    // ============================================================
    // HELPERS
    // ============================================================

    void removeDeadFromVector(std::vector<Actor*>& vec, VFXManager& vfx) {
        for (std::vector<Actor*>::iterator it = vec.begin(); it != vec.end(); ) {
            if (!(*it)->alive) {
                // ¿Tiene animación de muerte?
                if ((*it)->deathTexture.id != 0) {
                    float spawnX = (*it)->collider.x + ((*it)->collider.w / 2.0f);
                    float spawnY = (*it)->collider.y + (*it)->collider.h;

                    Texture2D finalSprite = ((*it)->causeOfDeath == DEATH_FLIPPED) ? (*it)->deathSpriteFlipped : (*it)->deathTexture;

                    // DEATH_FLIPPED → trayectoria parabólica
                    VFXMovement movement = ((*it)->causeOfDeath == DEATH_FLIPPED)
                        ? VFX_PARABOLA
                        : (*it)->defaultDeathMovement;

                    vfx.spawn(spawnX, spawnY,
                        finalSprite,
                        (*it)->deathW,
                        (*it)->deathH,
                        (*it)->deathTimeInterval,
                        (*it)->deathFrames,
                        movement);
                }
                delete* it;
                it = vec.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    void clearVector(std::vector<Actor*>& vec) {
        for (size_t i = 0; i < vec.size(); i++) delete vec[i];
        vec.clear();
    }

    void clearBulletVector(std::vector<Bullet*>& vec) {
        for (size_t i = 0; i < vec.size(); i++) delete vec[i];
        vec.clear();
    }
};

#endif // ACTORMANAGER_H