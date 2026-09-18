#include "raylib.h"
#include <cmath>
#include <vector>
#include <string>

// archivos de infraestructura
#include "displaymanager.h"
#include "globalandfps.h"
#include "inputmanager.h"
#include "gamescreens.h"

// archivos de motor
#include "Camera.h"       
#include "color.h"        
#include "Collider.h"
#include "Bullet.h"
#include "Actor.h"
#include "TileMap.h"
#include "PhysicsSystem.h"      
#include "CollisionTileMap.h"
#include "CollisionActors.h"
#include "VFX.h"
#include "VFXManager.h"
#include "ActorManager.h"
#include "PlatformController.h"
#include "AIController.h"
#include "PlayerController.h"
#include "ActorFactory.h"
#include "LevelManager.h"


int main() {

    // ============================================================
    // INIT
    // ============================================================
    DisplayManager dm;
    dm.init(SCREEN_WIDTH, SCREEN_HEIGHT, "Super Mario Bros Clone", DISPLAY_WINDOWED);
    SetTargetFPS(FPS_SPEED);

    InitAudioDevice();
    initFPSManager();   // no-op en raylib; conservado por compatibilidad

    // ============================================================
    // SONIDOS: LoadSound() necesita InitAudioDevice() activo.
    // ============================================================
    Sound jumpSnd = LoadSound("files/sounds/marioJumpSound.wav");
    Sound fireballSnd = LoadSound("files/sounds/fireballSound.wav");
    Sound koSound = LoadSound("files/sounds/marioStomp.wav");
    Sound hitSound = LoadSound("files/sounds/hit.wav");
    Sound clankSound = LoadSound("files/sounds/clank.wav");
    Sound lupSound = LoadSound("files/sounds/mario1up.wav");
    Sound coinSound = LoadSound("files/sounds/coinSound.wav");
    Sound shrinkSound = LoadSound("files/sounds/marioPowerDownSound.wav");
    Sound powerUpEmerging = LoadSound("files/sounds/pupemerging.wav");
    Sound brickShatterSnd = LoadSound("files/sounds/brickshattersound.wav");
    Sound brickMuffSnd = LoadSound("files/sounds/brickmuffsound.wav");
    Sound mushUpSound = LoadSound("files/sounds/mushroomUp.wav");

    // ============================================================
    Music bgMusic = LoadMusicStream("files/music/modernus.mp3");
    bgMusic.looping = true;
    PlayMusicStream(bgMusic);

    // ============================================================
    // TEXTURAS GLOBALES (lo que en Allegro eran BITMAPS)
    // ============================================================

    // Mario pequeño
    Texture2D p1Idle = LoadTexture("files/cuphead/marioIdle_.png");
    Texture2D p1Run = LoadTexture("files/cuphead/marioWalk_.png");
    Texture2D p1Jump = LoadTexture("files/cuphead/marioJump_.png");
    Texture2D p1Gun = LoadTexture("files/cuphead/cupheadGun.png");
    Texture2D p1RunGun = LoadTexture("files/cuphead/cupheadRunGun.png");
    Texture2D p1Dash = LoadTexture("files/cuphead/cupheadDashTest.png");
    Texture2D p1Crouch = LoadTexture("files/cuphead/marioDuck.png");
    Texture2D p1Drift = LoadTexture("files/cuphead/marioDrift_.png");
    Texture2D p1Defeat = LoadTexture("files/cuphead/marioDefeat_.png");

    // Mario grande
    Texture2D p1SuperRun = LoadTexture("files/cuphead/superMarioWalk_.png");
    Texture2D p1SuperIdle = LoadTexture("files/cuphead/superMarioIdle_.png");
    Texture2D p1SuperJump = LoadTexture("files/cuphead/superMarioJump_.png");
    Texture2D p1SuperCrouch = LoadTexture("files/cuphead/superMarioCrouch_.png");
    Texture2D p1SuperDrift = LoadTexture("files/cuphead/superMarioDrift_.png");

    // Mario fuego
    Texture2D p1FireRun = LoadTexture("files/cuphead/fireMarioWalk_.png");
    Texture2D p1FireIdle = LoadTexture("files/cuphead/fireMarioIdle_.png");
    Texture2D p1FireJump = LoadTexture("files/cuphead/fireMarioJump_.png");
    Texture2D p1FireCrouch = LoadTexture("files/cuphead/fireMarioCrouch_.png");
    Texture2D p1FireDrift = LoadTexture("files/cuphead/fireMarioDrift_.png");

    // ============================================================
    // INPUTS
    // ============================================================
    PlayerInputManager p1Input(InputSource::KEYBOARD_P1);
    PlayerInputManager p2Input(InputSource::KEYBOARD_P2);

#define ENTER_SAFE (p1Input.btn.start.safe)

    // ============================================================
    // PANTALLAS DE UI
    // ============================================================
    GameScreens screens(SCREEN_WIDTH, SCREEN_HEIGHT);

    // ============================================================
    // MOTOR DEL JUEGO
    // ============================================================
    GameCamera              cam(SCREEN_WIDTH, SCREEN_HEIGHT);
    TileMap                 map;
    ActorManager            actorManager;
    VFXManager              vfxManager;
    LevelManager            levelManager;
    std::vector<AIController>       enemyAIList;
    std::vector<PlatformController> platformControllers;
    std::vector<Actor*>             allActors;

    PlayerController p1Controller(NULL, 1);
    PlayerController p2Controller(NULL, 2);

    // Fondo del nivel (se repite horizontalmente)
    Texture2D bgTexture = { 0 };

    // ============================================================
    // MÁQUINA DE ESTADOS
    // ============================================================
    enum GameState {
        STATE_MENU,
        STATE_LOAD_LEVEL,
        STATE_PLAYING,
        STATE_WIN,
        STATE_GAMEOVER,
        STATE_GAMEBEATEN
    };
    GameState currentState = STATE_MENU;

    int currentLevel = 1;
    const int MAX_LEVELS = 2;
    const char* levelFiles[] = {
        "files/levels/level1.txt",
        "files/levels/level2.txt"
    };

    bool done = false;

    // ============================================================
    // GAME LOOP
    // ============================================================
    while (!dm.shouldClose() && !done) {

        // Input (una vez por frame)
        dm.handleInput();
        p1Input.update(p2Input);
        p2Input.update(p1Input);

        // Acumula tiempo del frame
        tickAccumulator();

        // ============================================================
        // UPDATE
        // ============================================================
        while (hasPendingTicks()) {

            UpdateMusicStream(bgMusic);

            if (IsKeyDown(KEY_ESCAPE)) done = true;

            switch (currentState) {

                // MENU
            case STATE_MENU:
                if (ENTER_SAFE) {
                    currentLevel = 1;
                    currentState = STATE_LOAD_LEVEL;
                }
                break;

                // CARGA DE NIVEL
            case STATE_LOAD_LEVEL:
            {
                // Limpiar estado anterior
                actorManager.clearAll();
                enemyAIList.clear();
                platformControllers.clear();
                levelManager.clear();
                if (bgTexture.id != 0) { UnloadTexture(bgTexture); bgTexture = { 0 }; }

                // Cargar nivel desde archivo
                if (!levelManager.loadLevel(levelFiles[currentLevel - 1],
                    actorManager, map,
                    enemyAIList, platformControllers))
                {
                    // Nivel no encontrado: ir a game over en lugar de crashear
                    currentState = STATE_GAMEOVER;
                    break;
                }

                // Fondo del nivel
                if (!levelManager.backgroundPath.empty())
                    bgTexture = LoadTexture(levelManager.backgroundPath.c_str());

                // Límites de cámara
                cam.setLimits(levelManager.mapWidth * TILE_SIZE,
                    levelManager.mapHeight * TILE_SIZE);

                // Vestimos a P1 con todas sus animaciones
                if (levelManager.p1) {
                    Actor* p1 = levelManager.p1;

                    // Mario pequeño
                    p1->addAnimation("SMALL_IDLE", p1Idle, 100, 8, 1, ANIM_LOOP);
                    p1->addAnimation("SMALL_RUN", p1Run, 100, 7, 3, ANIM_LOOP);
                    p1->addAnimation("SMALL_TURBORUN", p1Run, 100, 5, 3, ANIM_LOOP);
                    p1->addAnimation("SMALL_DRIFTING", p1Drift, 100, 5, 1, ANIM_LOOP);
                    p1->addAnimation("SMALL_JUMP", p1Jump, 100, 20, 1, ANIM_ONCE);
                    p1->addAnimation("SMALL_SHOOT_IDLE", p1Gun, 100, 7, 2, ANIM_ONCE);
                    p1->addAnimation("SMALL_RUN_SHOOT", p1RunGun, 100, 7, 4, ANIM_LOOP);
                    p1->addAnimation("SMALL_DASH", p1Dash, 100, 4, 1, ANIM_LOOP);
                    p1->addAnimation("SMALL_CROUCH_IDLE", p1Crouch, 100, 7, 1, ANIM_ONCE);
                    p1->addAnimation("SMALL_CROUCH_SHOOT", p1Crouch, 100, 7, 2, ANIM_ONCE);
                    p1->addAnimation("SMALL_PUSH", p1Run, 100, 7, 3, ANIM_LOOP);
                    p1->addAnimation("DEATH", p1Defeat, 100, 7, 1, ANIM_LOOP);

                    // Mario grande
                    p1->addAnimation("BIG_IDLE", p1SuperIdle, 100, 8, 1, ANIM_LOOP);
                    p1->addAnimation("BIG_RUN", p1SuperRun, 100, 7, 3, ANIM_LOOP);
                    p1->addAnimation("BIG_TURBORUN", p1SuperRun, 100, 5, 3, ANIM_LOOP);
                    p1->addAnimation("BIG_DRIFTING", p1SuperDrift, 100, 5, 1, ANIM_LOOP);
                    p1->addAnimation("BIG_JUMP", p1SuperJump, 100, 20, 1, ANIM_ONCE);
                    p1->addAnimation("BIG_SHOOT_IDLE", p1Gun, 100, 7, 2, ANIM_ONCE);
                    p1->addAnimation("BIG_RUN_SHOOT", p1RunGun, 100, 7, 4, ANIM_LOOP);
                    p1->addAnimation("BIG_DASH", p1Dash, 100, 4, 1, ANIM_LOOP);
                    p1->addAnimation("BIG_CROUCH_IDLE", p1SuperCrouch, 100, 7, 1, ANIM_ONCE);
                    p1->addAnimation("BIG_CROUCH_SHOOT", p1SuperCrouch, 100, 7, 2, ANIM_ONCE);
                    p1->addAnimation("BIG_PUSH", p1SuperRun, 100, 7, 3, ANIM_LOOP);

                    // Mario fuego
                    p1->addAnimation("FIRE_IDLE", p1FireIdle, 100, 8, 1, ANIM_LOOP);
                    p1->addAnimation("FIRE_RUN", p1FireRun, 100, 7, 3, ANIM_LOOP);
                    p1->addAnimation("FIRE_TURBORUN", p1FireRun, 100, 5, 3, ANIM_LOOP);
                    p1->addAnimation("FIRE_DRIFTING", p1FireDrift, 100, 5, 1, ANIM_LOOP);
                    p1->addAnimation("FIRE_JUMP", p1FireJump, 100, 20, 1, ANIM_ONCE);
                    p1->addAnimation("FIRE_SHOOT_IDLE", p1Gun, 100, 7, 2, ANIM_ONCE);
                    p1->addAnimation("FIRE_RUN_SHOOT", p1RunGun, 100, 7, 4, ANIM_LOOP);
                    p1->addAnimation("FIRE_DASH", p1Dash, 100, 4, 1, ANIM_LOOP);
                    p1->addAnimation("FIRE_CROUCH_IDLE", p1FireCrouch, 100, 7, 1, ANIM_ONCE);
                    p1->addAnimation("FIRE_CROUCH_SHOOT", p1FireCrouch, 100, 7, 2, ANIM_ONCE);
                    p1->addAnimation("FIRE_PUSH", p1FireRun, 100, 7, 3, ANIM_LOOP);

                    p1Controller.actor = p1;
                }

                currentState = STATE_PLAYING;
                break;
            }

            // VICTORIA
            case STATE_WIN:
                if (ENTER_SAFE) {
                    currentLevel++;
                    currentState = (currentLevel > MAX_LEVELS)
                        ? STATE_GAMEBEATEN
                        : STATE_LOAD_LEVEL;
                }
                break;

                // DERROTA
            case STATE_GAMEOVER:
                if (ENTER_SAFE) currentState = STATE_LOAD_LEVEL;
                break;

                // JUEGO COMPLETO
            case STATE_GAMEBEATEN:
                if (ENTER_SAFE) {
                    currentLevel = 1;
                    currentState = STATE_MENU;
                }
                break;

                // JUGANDO
            case STATE_PLAYING:
            {
                allActors = actorManager.getAllActors();
                int actorCount = (int)allActors.size();

                std::vector<Actor*>& players = actorManager.getPlayers();
                std::vector<Actor*>& enemies = actorManager.getEnemies();
                std::vector<Actor*>& obstacles = actorManager.getObstacles();
                std::vector<Actor*>& platforms = actorManager.getPlatforms();
                std::vector<Actor*>& items = actorManager.getItems();
                std::vector<Actor*>& fireballs = actorManager.getPlayerFireballs();

                Actor* p1 = levelManager.p1;

                // Hitstop 
                // Congela la simulación durante transformaciones y muerte cinemática.
                bool gameFrozen = p1 && (p1->isTransforming || p1->isDying);

                if (gameFrozen) {
                    p1->hitstopTimer--;

                    if (p1->isTransforming) {
                        // Extrae el sufijo de la animación actual (sin prefijo)
                        const std::string& cur = p1->currentAnimName;
                        std::string suffix = "IDLE";
                        if (cur.find("SMALL_") == 0) suffix = cur.substr(6);
                        else if (cur.find("BIG_") == 0) suffix = cur.substr(4);
                        else if (cur.find("FIRE_") == 0) suffix = cur.substr(5);

                        // Alterna prefijos cada 4 frames → efecto visual de transformación
                        std::string activePrefix = ((p1->hitstopTimer / 4) % 2 == 0)
                            ? p1->targetAnimPrefix
                            : p1->oldAnimPrefix;
                        p1->playAnimation(activePrefix + suffix);

                        if (p1->hitstopTimer <= 0) {
                            p1->isTransforming = false;
                            p1->animPrefix = p1->targetAnimPrefix;
                            if (p1->animPrefix == "SMALL_") p1->changePhysicalSize(25, 25);
                            else                            p1->changePhysicalSize(25, 50);
                        }
                    }
                    else if (p1->isDying) {
                        p1->playAnimation("DEATH");
                        if (p1->hitstopTimer == 150) {
                            p1->vy = -10.0f;
                        }
                        else if (p1->hitstopTimer < 150) {
                            p1->vy += 0.5f;
                            p1->y += p1->vy;
                        }
                        if (p1->hitstopTimer <= 0) currentState = STATE_GAMEOVER;
                    }

                    for (int i = 0; i < actorCount; i++) allActors[i]->updateSprite();
                }
                else {
                    // Controladores de jugador
                    p1Controller.update(p1Input.btn, map, allActors, jumpSnd, { 0 });
                    // activar para segundo player (cerciorar de que exista)
                    // p2Controller.update(p2Input.btn, map, allActors, jumpSnd, {0});

                    // Spawn de bola de fuego
                    if (p1 && p1->wantsToShootFireball) {
                        p1->wantsToShootFireball = false;
                        if (actorManager.getPlayerFireballs().size() < 2) {
                            float fbX = p1->lookingLeft
                                ? p1->collider.x - 16
                                : p1->collider.x + p1->collider.w;
                            float fbY = p1->collider.y + 10;

                            Actor* fb = ActorFactory::spawn("PROJECTILE_FIREBALL",
                                fbX, fbY,
                                actorManager,
                                enemyAIList,
                                platformControllers,
                                levelManager.levelSprites);
                            if (fb) {
                                fb->targetDirection = p1->lookingLeft ? -1 : 1;
                                fb->vy = 2.0f;
                                if (fireballSnd.frameCount > 0) PlaySound(fireballSnd);
                            }
                        }
                    }

                    // Guardar posiciones previas
                    savePreviousPositions(&allActors[0], actorCount);

                    // Plataformas y IA
                    for (size_t i = 0; i < platformControllers.size(); i++)
                        platformControllers[i].update();

                    for (size_t i = 0; i < enemyAIList.size(); i++) {
                        enemyAIList[i].update(p1, actorManager,
                            fireballSnd, coinSound,
                            powerUpEmerging,
                            brickShatterSnd, brickMuffSnd,
                            cam, map);
                    }

                    // Física
                    applyGravityToAll(&allActors[0], actorCount);

                    // Carriers (plataformas) se mueven primero
                    moveCarriersX(&allActors[0], actorCount);
                    moveCarriersY(&allActors[0], actorCount);
                    for (size_t i = 0; i < platforms.size(); i++) {
                        resolveActorVsTilemapX(platforms[i], map);
                        resolveActorVsTilemapY(platforms[i], map);
                    }

                    applyCarrierMovement(&allActors[0], actorCount);
                    resetGroundedState(&allActors[0], actorCount);
                    resetCarryStates(&allActors[0], actorCount);

                    for (int i = 0; i < actorCount; i++) allActors[i]->isPushing = false;

                    // Bullets
                    actorManager.updateBullets();
                    actorManager.removeOffscreenBullets(levelManager.mapWidth * TILE_SIZE,
                        levelManager.mapHeight * TILE_SIZE);
                    actorManager.checkBulletTilemapCollisions(map);
                    actorManager.checkBulletObstacleCollisions(hitSound, koSound);
                    actorManager.checkBulletCollisions(hitSound, koSound, clankSound);

                    // Colisiones de actores
                    actorManager.checkPlayerEnemyCollisions(hitSound, koSound, shrinkSound);
                    actorManager.checkPlayerItemCollisions(mushUpSound, lupSound, coinSound);
                    actorManager.checkObstacleEnemyCollisions(koSound);
                    actorManager.checkObstacleItemCollisions();
                    actorManager.checkFireballEnemyCollisions(koSound);
                    actorManager.checkEnemyEnemyCollisions(koSound);

                    // Zona de muerte
                    float mapBottom = (levelManager.mapHeight * TILE_SIZE) + 150.0f;
                    float mapRight = (levelManager.mapWidth * TILE_SIZE) + 200.0f;
                    actorManager.checkOutOfBounds(-200.0f, mapRight, mapBottom);

                    // Condiciones de victoria / derrota
                    if (p1) {
                        if (!p1->alive) {
                            currentState = STATE_GAMEOVER;
                        }
                        else {
                            // Colisión con el goal
                            for (size_t i = 0; i < obstacles.size(); i++) {
                                if (obstacles[i]->name == "Goal" &&
                                    checkAABB(p1->collider.x, p1->collider.y,
                                        p1->collider.w, p1->collider.h,
                                        obstacles[i]->collider.x, obstacles[i]->collider.y,
                                        obstacles[i]->collider.w, obstacles[i]->collider.h))
                                {
                                    currentState = STATE_WIN;
                                }
                            }
                        }
                        p1->updateShootingState();
                        p1->updateDashingState();
                        p1->updateInvincibility();
                    }

                    // Movimiento de no-carriers
                    moveNonCarriersX(&allActors[0], actorCount);
                    for (size_t i = 0; i < players.size(); i++) resolveActorVsTilemapX(players[i], map);
                    for (size_t i = 0; i < obstacles.size(); i++) resolveActorVsTilemapX(obstacles[i], map);
                    for (size_t i = 0; i < enemies.size(); i++) resolveActorVsTilemapX(enemies[i], map);
                    for (size_t i = 0; i < items.size(); i++) resolveActorVsTilemapX(items[i], map);

                    for (size_t i = 0; i < fireballs.size(); i++) {
                        resolveActorVsTilemapX(fireballs[i], map);
                        for (size_t o = 0; o < obstacles.size(); o++)
                            resolveActorVsActorX(fireballs[i], obstacles[o], map, &allActors[0], actorCount, true);
                    }
                    for (size_t p = 0; p < players.size(); p++) {
                        for (size_t o = 0; o < obstacles.size(); o++)
                            resolveActorVsActorX(players[p], obstacles[o], map, &allActors[0], actorCount);
                        for (size_t f = 0; f < platforms.size(); f++)
                            resolveActorVsActorX(players[p], platforms[f], map, &allActors[0], actorCount);
                    }
                    for (size_t e = 0; e < enemies.size(); e++) {
                        for (size_t o = 0; o < obstacles.size(); o++)
                            resolveActorVsActorX(enemies[e], obstacles[o], map, &allActors[0], actorCount, true);
                    }
                    for (size_t i = 0; i < items.size(); i++) {
                        if (items[i]->powerUpType == 0) continue;
                        for (size_t o = 0; o < obstacles.size(); o++)
                            resolveActorVsActorX(items[i], obstacles[o], map, &allActors[0], actorCount, true);
                    }

                    moveNonCarriersY(&allActors[0], actorCount);
                    for (size_t i = 0; i < players.size(); i++) resolveActorVsTilemapY(players[i], map);
                    for (size_t i = 0; i < obstacles.size(); i++) resolveActorVsTilemapY(obstacles[i], map);
                    for (size_t i = 0; i < enemies.size(); i++) resolveActorVsTilemapY(enemies[i], map);
                    for (size_t i = 0; i < items.size(); i++) resolveActorVsTilemapY(items[i], map);

                    // Sonido de cabeza contra bloque
                    if (p1 && p1->bumpedHead) {
                        if (brickMuffSnd.frameCount > 0) PlaySound(brickMuffSnd);
                        p1->bumpedHead = false;
                    }

                    for (size_t i = 0; i < fireballs.size(); i++) {
                        resolveActorVsTilemapY(fireballs[i], map);
                        for (size_t o = 0; o < obstacles.size(); o++)
                            resolveActorVsActorY(fireballs[i], obstacles[o], true);
                    }
                    for (size_t p = 0; p < players.size(); p++) {
                        for (size_t o = 0; o < obstacles.size(); o++) resolveActorVsActorY(players[p], obstacles[o]);
                        for (size_t f = 0; f < platforms.size(); f++) resolveActorVsActorY(players[p], platforms[f]);
                        for (size_t e = 0; e < enemies.size(); e++) resolveActorVsActorY(players[p], enemies[e]);
                    }
                    for (size_t e = 0; e < enemies.size(); e++) {
                        for (size_t o = 0; o < obstacles.size(); o++)
                            resolveActorVsActorY(enemies[e], obstacles[o], true);
                    }
                    for (size_t i = 0; i < items.size(); i++) {
                        if (items[i]->powerUpType == 0) continue;
                        resolveActorVsTilemapY(items[i], map);
                        for (size_t o = 0; o < obstacles.size(); o++)
                            resolveActorVsActorY(items[i], obstacles[o], true);
                    }

                    updateJumpState(&allActors[0], actorCount);

                    // Cámara, sprites, VFX
                    if (p1) cam.update(p1->x, p1->y);
                    for (int i = 0; i < actorCount; i++) allActors[i]->updateSprite();

                    // Limpiar AIControllers de enemigos muertos
                    for (std::vector<AIController>::iterator it = enemyAIList.begin();
                        it != enemyAIList.end(); )
                    {
                        if (!it->enemy->alive) it = enemyAIList.erase(it);
                        else ++it;
                    }

                    actorManager.removeDeadActors(vfxManager);
                    vfxManager.update();
                }
                break;
            } // fin case STATE_PLAYING

            } // fin switch currentState

            cerrarCicloWhile();
        } // fin while hasPendingTicks


        // ============================================================
        // RENDER
        // ============================================================
        dm.beginVirtualDraw();
        ClearBackground(BLACK);

        switch (currentState) {

        case STATE_MENU:       screens.draw(SCREEN_MENU);       break;
        case STATE_WIN:        screens.draw(SCREEN_WIN);        break;
        case STATE_GAMEOVER:   screens.draw(SCREEN_GAMEOVER);   break;
        case STATE_GAMEBEATEN: screens.draw(SCREEN_GAMEBEATEN); break;

        case STATE_PLAYING:
        {
            int sx = (int)cam.x;
            int sy = (int)cam.y;

            // Fondo (repetido horizontalmente)
            if (bgTexture.id != 0) {
                int bgW = bgTexture.width;
                int bgH = bgTexture.height;
                int mapPixelW = levelManager.mapWidth * TILE_SIZE;
                for (int bx = 0; bx < mapPixelW; bx += bgW) {
                    int drawX = bx - sx;
                    int drawY = -sy;
                    if (drawX + bgW > 0 && drawX < SCREEN_WIDTH)
                        DrawTexture(bgTexture, drawX, 0 /*usar drawY si quieres scroll Vertical*/, WHITE);
                }
            }

            // Tilemap
            map.draw(sx, sy);

            // Actores (orden de profundidad manejado por ActorManager)
            actorManager.drawAll(cam);

            // Efectos VFX
            vfxManager.drawAll(sx, sy);

            // Debug info del jugador
            if (levelManager.p1) levelManager.p1->debugInfo(10, 260);

            // Debug colliders (descomentar para depurar):
            actorManager.drawAllColliders(cam, BLUE);
            map.drawDebugGrid(sx, sy);

            break;
        }

        default: break;
        } // switch currentState

       // DEBUGS:
        inputDebug(p1Input.btn, 10, 30, false);
        inputDebug(p2Input.btn, 420, 30, false);

        mostrarFPS(10, 820);
        DrawText(dm.getScaleInfo(), 10, 900, 10, LIME); // escala actual

        dm.endVirtualDraw();

    } // fin while game loop


    // ============================================================
    // LIMPIEZA
    // ============================================================
    if (bgTexture.id != 0) UnloadTexture(bgTexture);

    // Texturas de jugador
    UnloadTexture(p1Idle);    UnloadTexture(p1Run);     UnloadTexture(p1Jump);
    UnloadTexture(p1Gun);     UnloadTexture(p1RunGun);  UnloadTexture(p1Dash);
    UnloadTexture(p1Crouch);  UnloadTexture(p1Drift);   UnloadTexture(p1Defeat);
    UnloadTexture(p1SuperRun); UnloadTexture(p1SuperIdle); UnloadTexture(p1SuperJump);
    UnloadTexture(p1SuperCrouch); UnloadTexture(p1SuperDrift);
    UnloadTexture(p1FireRun); UnloadTexture(p1FireIdle); UnloadTexture(p1FireJump);
    UnloadTexture(p1FireCrouch); UnloadTexture(p1FireDrift);

    // Sonidos
    UnloadSound(jumpSnd);      UnloadSound(fireballSnd);  UnloadSound(koSound);
    UnloadSound(hitSound);     UnloadSound(clankSound);   UnloadSound(lupSound);
    UnloadSound(coinSound);    UnloadSound(shrinkSound);  UnloadSound(powerUpEmerging);
    UnloadSound(brickShatterSnd); UnloadSound(brickMuffSnd); UnloadSound(mushUpSound);

    UnloadMusicStream(bgMusic);

    screens.destroy();
    dm.destroy();
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
