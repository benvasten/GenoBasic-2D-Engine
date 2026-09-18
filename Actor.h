#ifndef ACTOR_H
#define ACTOR_H

#include "raylib.h"
#include "color.h" 
#include <string>
#include <map>
#include "collider.h"
#include "bullet.h"

#define MAX_DASH_SPEED 5.5f


enum MushRoomBlockState {
    NOTHIT,
    BEINGHIT,
    ALREADYHIT
};

enum AimDir { NONE, UP, DOWN, LEFT, RIGHT, UP_RIGHT, UP_LEFT, DOWN_RIGHT, DOWN_LEFT };

enum AnimPlayback {
    ANIM_ONCE,      // reproduce 1 vez y hace hold en ultimo frame
    ANIM_LOOP,      
    ANIM_PINGPONG   
};

enum VFXMovement {
    VFX_STATIC,
    VFX_FALL_DOWN,
    VFX_PARABOLA,
    VFX_UP_AND_DOWN
};

enum DeathCause {
    DEATH_SQUASHED,
    DEATH_FLIPPED
};

struct Animation {
    Texture2D texture;         // posición defecto normal no flipped: mirando derecha
    Texture2D textureFlipped;  
    int frameWidth;
    int timeInterval;
    int numFrames;
    AnimPlayback type;

    Animation()
        : frameWidth(0), timeInterval(0), numFrames(0), type(ANIM_ONCE)
    {
        texture = { 0 };
        textureFlipped = { 0 };
    }

    Animation(Texture2D tex, Texture2D texFlip, int w, int t, int n, AnimPlayback pb)
        : texture(tex), textureFlipped(texFlip),
        frameWidth(w), timeInterval(t), numFrames(n), type(pb) {}

    bool isValid() const { return texture.id != 0; }
};


// ============================================================
// ACTOR
// ============================================================
class Actor {
public:
    // ----------------------------------------
    // POSICION Y FISICA BASICA (donde esta, a donde va)
    // ----------------------------------------
    float x, y;
    int w, h;
    Collider collider;
    ColliderType colliderType;
    SphereCollider sphere;
    float sphereOffsetX;
    float sphereOffsetY;
    float prevX, prevY;

    int originalHeight;
    int crouchHeight;
    bool isCrouching;

    int visualW;
    int visualH;

    float vx, vy;
    float intendedVy;
    float maxSpeed;
    float acceleration;
    float deceleration;
    float targetVx;

    bool turboRunning;
    float turboIncrFactor;
    float turboMaxJumpIncrease;
    int turboHoldCount;

    bool affectedByGravity;
    bool lookingLeft;
    bool isGrounded;
    bool running;
    bool isDrifting;
    bool isAwake;

    // ----------------------------------------
    // SALTO
    // ----------------------------------------
    float jumpForce;
    float jumpHoldForce;
    int jumpHoldFrames;
    int currentJumpFrames;
    bool canJump;
    bool isJumping;

    // ----------------------------------------
    // DISPARO Y APUNTADO
    // ----------------------------------------
    bool isShootingHold;
    bool justShot;
    int currentShotFrames;
    int aimX, aimY;
    bool wantsToShootFireball;

    // ----------------------------------------
    // DASH
    // ----------------------------------------
    bool justDash;
    int currentDashFrames;
    bool hasDashedInAir;

    // ----------------------------------------
    // ESTADOS Y FLAGS
    // ----------------------------------------
    bool alive;
    int vidas;

    bool isPushing;
    bool isAnchored;
    bool crouchMoveLock;

    bool isInvincible;
    int currentInvincibilityFrames;
    int maxInvincibilityFrames;

    bool isStarInvincible;
    int starTimer;

    bool isTransforming;
    bool isDying;
    int hitstopTimer;
    std::string oldAnimPrefix;
    std::string targetAnimPrefix;

    bool bumpedHead;

    // ----------------------------------------
    // COLISION DE INTERACCION 
    // ----------------------------------------
    bool canCarry;
    Actor* carriedBy;
    bool canPush;
    bool isPushable;
    bool useInertia;
    bool isSolid;

    // ----------------------------------------
    // BANDERAS DE BEHAVIOR
    // ----------------------------------------
    bool isStompable;
    bool immuneToBullets;
    bool isLavaFloored;
    bool dealsDamage;
    bool isShell;

    int  kickGracePeriod;  
    int  postStompGracePeriod;

    int powerUpType; // 0=nada, 1=champiñón, 2=flor, 3=moneda, 4=1up, 5=estrella

    bool isHitableFromBelow;
    MushRoomBlockState beenHitAsBlock;
    int beenHitCount;
    float originalBlock_Y;

    bool isHidden;
    bool isStored;
    Actor* storedItem;

    float lastHitByX;
    int targetDirection;
    bool hitByBigMario;
    int multiTimer;

    // ----------------------------------------
    // MUERTE Y VFX
    // ----------------------------------------
    Texture2D deathTexture;     // <- es spritesheet
    Texture2D deathSpriteFlipped;
    int deathFrames;
    int deathTimeInterval;
    int deathW;
    int deathH;
    DeathCause causeOfDeath;
    VFXMovement defaultDeathMovement;

    // ----------------------------------------
    // ANIMACIONES
    // ----------------------------------------
    std::map<std::string, Animation> animations;
    std::string currentAnimName;
    int animTimer;
    int currentFrameIndex;
    int animDirection;
    bool animFinished;
    std::string animPrefix;     // para (de mario): "SMALL_", "BIG_", "FIRE_"

    // ----------------------------------------
    // Constructor
    // ----------------------------------------
    std::string name;
    // ============================================================
    Actor(float x_, float y_, int w_, int h_, EngineColor /*color*/, const std::string& name_,
        int visualW_, int visualH_,
        bool gravity_ = true, bool canCarry_ = false,
        bool canPush_ = false, bool isPushable_ = false)
        : x(x_), y(y_), w(w_), h(h_),
        collider(x_, y_, w_, h_),
        colliderType(COLLIDER_AABB),
        sphereOffsetX(0), sphereOffsetY(0),
        visualW(visualW_), visualH(visualH_),
        vx(0), vy(0), intendedVy(0), prevX(x_), prevY(y_), name(name_),
        maxSpeed(4.0f), acceleration(0.5f), deceleration(0.8f), targetVx(0),
        turboRunning(false), turboIncrFactor(1.0f), turboMaxJumpIncrease(2.0f),
        turboHoldCount(0),
        isDrifting(false),
        isLavaFloored(false), hitByBigMario(false),
        affectedByGravity(gravity_), isGrounded(false), running(false), lookingLeft(false),
        isShootingHold(false), justShot(false), currentShotFrames(0), aimX(0), aimY(0),
        isAwake(true),
        justDash(false), currentDashFrames(0), hasDashedInAir(false),
        isStompable(false), immuneToBullets(false),
        lastHitByX(0), targetDirection(1),
        powerUpType(0),
        multiTimer(-1),
        isHitableFromBelow(false), beenHitAsBlock(NOTHIT), beenHitCount(0), originalBlock_Y(0),
        alive(true), vidas(1),
        isPushing(false), isAnchored(false), useInertia(false),
        crouchMoveLock(false),
        canCarry(canCarry_), carriedBy(NULL), canPush(canPush_), isPushable(isPushable_), isSolid(true),
        jumpForce(8.0f), jumpHoldForce(0.3f), jumpHoldFrames(10), currentJumpFrames(0),
        canJump(false), isJumping(false),
        isCrouching(false), originalHeight(h_), crouchHeight(h_ / 2),
        animTimer(0), currentFrameIndex(0), animFinished(false), animDirection(1),
        animPrefix("SMALL_"),
        isInvincible(false), currentInvincibilityFrames(0), maxInvincibilityFrames(60), postStompGracePeriod(0),
        isStarInvincible(false), starTimer(0),
        deathFrames(1), deathTimeInterval(5),
        deathW(visualW_), deathH(visualH_),
        wantsToShootFireball(false),
        causeOfDeath(DEATH_SQUASHED), defaultDeathMovement(VFX_STATIC),
        isHidden(false), isStored(false), storedItem(NULL),
        dealsDamage(true), isShell(false), kickGracePeriod(0),
        isTransforming(false), isDying(false), hitstopTimer(0),
        oldAnimPrefix(""), targetAnimPrefix(""),
        bumpedHead(false)
    {
        deathTexture = { 0 };
        deathSpriteFlipped = { 0 };
    }

    ~Actor() {
        for (std::map<std::string, Animation>::iterator it = animations.begin();
            it != animations.end(); ++it)
        {
            if (it->second.textureFlipped.id != 0) {
                UnloadTexture(it->second.textureFlipped);
                it->second.textureFlipped = { 0 };
            }
        }
    }

private:
    Actor(const Actor&);
    Actor& operator=(const Actor&);

public:
    // ============================================================
    // MOVIMIENTO
    // ============================================================

    void savePreviousPosition() {
        prevX = x;
        prevY = y;
        if (colliderType == COLLIDER_SPHERE) {
            sphere.prevCx = sphere.cx;
            sphere.prevCy = sphere.cy;
        }
    }

    void setSphereCollider(float radius, float offsetX, float offsetY) {
        colliderType = COLLIDER_SPHERE;
        sphere.radius = radius;
        sphereOffsetX = offsetX;
        sphereOffsetY = offsetY;
        syncCollider();
        sphere.prevCx = sphere.cx;
        sphere.prevCy = sphere.cy;
    }

    void setMovingInput(float dirX) {
        if (!justDash) {
            if (!useInertia) {
                // sin inercia es rigido. tipo Megaman / Cuphead
                float currentSpeed = isCrouching ? 1.5f : 3.0f;
                if (turboRunning) currentSpeed += turboIncrFactor;

                if (dirX < 0) { vx = -currentSpeed; lookingLeft = true; }
                else if (dirX > 0) { vx = currentSpeed; lookingLeft = false; }
                else { vx = 0; }

                running = (vx != 0);
            }
            else {
                // con inercia es tipo Mario
                float currentMaxSpeed = isCrouching ? (maxSpeed * 0.4f) : maxSpeed;
                if (turboRunning) currentMaxSpeed += turboIncrFactor;

                if (dirX < 0) {
                    vx -= acceleration;
                    if (vx < -currentMaxSpeed) vx = -currentMaxSpeed;
                    lookingLeft = true;
                }
                else if (dirX > 0) {
                    vx += acceleration;
                    if (vx > currentMaxSpeed) vx = currentMaxSpeed;
                    lookingLeft = false;
                }
                else {
                    // Para Mario, no hay input de desacelerar. turboRunning NO aumenta velocidad
                    // total cuando dirX==0, solo amplia la velocidad maxima cuando hay direccion.
                    if (vx > 0) { vx -= deceleration; if (vx < 0) vx = 0; }
                    else if (vx < 0) { vx += deceleration; if (vx > 0) vx = 0; }
                }

                running = (vx > 0.5f || vx < -0.5f);

                // turboHoldCount solo crece cuando hay direccion activa
                if (turboRunning && dirX != 0) turboHoldCount++;
                else                           turboHoldCount--;
                if (turboHoldCount > 10) turboHoldCount = 10;
                if (turboHoldCount < 0)  turboHoldCount = 0;
            }
        }
    }

    void setAim(int ax, int ay) { aimX = ax; aimY = ay; }

    void moveX() { x += vx; syncCollider(); }
    void moveY() { y += vy; syncCollider(); }

    void syncCollider() {
        if (colliderType == COLLIDER_SPHERE) {
            sphere.cx = x + sphereOffsetX;
            sphere.cy = y + sphereOffsetY;
            float bx, by;
            int bw, bh;
            sphereToAABB(sphere.cx, sphere.cy, sphere.radius, bx, by, bw, bh);
            collider.x = bx;
            collider.y = by;
            collider.w = bw;
            collider.h = bh;
        }
        else {
            collider.x = x;
            collider.y = y;
            collider.w = w;
            collider.h = h;
        }
    }

    void stopX() { vx = 0; }
    void stopY() { vy = 0; }

    // ============================================================
    // SALTO
    // ============================================================
    void jumpStart() {
        if (canJump && isGrounded) {
            float jumpForceToUse = jumpForce;

            if (vx > maxSpeed || vx < -maxSpeed) {
                float increase = jumpForce + turboHoldCount;
                if (increase >= turboMaxJumpIncrease) increase = turboMaxJumpIncrease;
                jumpForceToUse = jumpForce + increase;
            }

            vy = -jumpForceToUse;
            isJumping = true;
            canJump = false;
            currentJumpFrames = 0;
        }
    }

    void jumpHold() {
        if (isJumping && currentJumpFrames < jumpHoldFrames) {
            vy -= jumpHoldForce;
            currentJumpFrames++;
        }
        else {
            isJumping = false;
        }
    }

    void jumpStop() {
        isJumping = false;
        if (vy < 0) vy *= 0.5f;
    }

    // ============================================================
    // CROUCH
    // ============================================================
    void crouch() {
        if (!isCrouching) {
            isCrouching = true;
            y += (originalHeight - crouchHeight);
            h = crouchHeight;
            syncCollider();
            crouchMoveLock = true;
        }
    }

    void standUp() {
        if (isCrouching) {
            isCrouching = false;
            y -= (originalHeight - crouchHeight);
            h = originalHeight;
            syncCollider();
            crouchMoveLock = false;
        }
    }

    // ============================================================
    // TRANSFORMACIÓN DE TAMAÑO
    // ============================================================
    void changePhysicalSize(int newW, int newH) {
        int deltaH = newH - h;
        y -= deltaH;
        w = newW;
        h = newH;
        originalHeight = newH;
        crouchHeight = newH / 2;
        syncCollider();
    }

    // ============================================================
    // INVENCIBILIDAD
    // ============================================================
    void triggerInvincibility() {
        isInvincible = true;
        currentInvincibilityFrames = maxInvincibilityFrames;
    }

    void updateInvincibility() {
        if (isInvincible) {
            currentInvincibilityFrames--;
            if (currentInvincibilityFrames <= 0) {
                isInvincible = false;
                currentInvincibilityFrames = 0;
            }
        }
        if (postStompGracePeriod > 0) postStompGracePeriod--;
        if (isStarInvincible) {
            starTimer--;
            if (starTimer <= 0) {
                isStarInvincible = false;
                starTimer = 0;
            }
        }
    }

    // ============================================================
    // DASH
    // ============================================================
    void updateDashingState() {
        if (justDash) {
            currentDashFrames++;
            if (aimX > 0) vx = MAX_DASH_SPEED;
            else if (aimX < 0) vx = -MAX_DASH_SPEED;
            else               vx = lookingLeft ? -MAX_DASH_SPEED : MAX_DASH_SPEED;
            vy = 0;
            affectedByGravity = false;
        }
        if (currentDashFrames >= 25) {
            currentDashFrames = 0;
            justDash = false;
            affectedByGravity = true;
        }
    }

    // ============================================================
    // DISPARO
    // ============================================================
    void updateShootingState() {
        if (justShot) currentShotFrames++;
        if (currentShotFrames >= 40) {
            justShot = false;
            currentShotFrames = 0;
        }
    }

    Bullet* shoot(const char* bulletSpritePath) {
        justShot = true;
        currentShotFrames = 0;
        int bulletW = 24;
        int bulletH = 9;
        float speedMag = 12.0f;
        float bulletVx = 0.0f;
        float bulletVy = 0.0f;

        if (aimX == 0 && aimY == 0) {
            bulletVx = lookingLeft ? -speedMag : speedMag;
            bulletVy = 0.0f;
        }
        else {
            bulletVx = aimX * speedMag;
            bulletVy = aimY * speedMag;
        }

        float bulletY = y + (h / 2) - bulletH * 2;
        float bulletX = x;
        return new Bullet(bulletX, bulletY, bulletW, bulletH, bulletSpritePath, bulletVx, bulletVy);
    }

    // ============================================================
    // ANIMACIONES
    // ============================================================
  
    void addAnimation(const std::string& animName, Texture2D tex,
        int fWidth, int tInterval, int nFrames, AnimPlayback type)
    {
        Texture2D texFlipped = { 0 };

        if (tex.id != 0 && IsWindowReady()) {
            Image sheetImg = LoadImageFromTexture(tex);

            if (sheetImg.data != NULL) {
                int fh = sheetImg.height;
                Image flippedSheet = GenImageColor(sheetImg.width, fh, BLANK);

                for (int i = 0; i < nFrames; i++) {
                    Rectangle frameSrc = { (float)(fWidth * i), 0.0f, (float)fWidth, (float)fh };
                    Image frameImg = ImageFromImage(sheetImg, frameSrc);
                    ImageFlipHorizontal(&frameImg);
                    Rectangle frameDst = { (float)(fWidth * i), 0.0f, (float)fWidth, (float)fh };
                    ImageDraw(&flippedSheet, frameImg,
                        { 0, 0, (float)fWidth, (float)fh },
                        frameDst, WHITE);
                    UnloadImage(frameImg);
                }

                texFlipped = LoadTextureFromImage(flippedSheet);
                UnloadImage(flippedSheet);
                UnloadImage(sheetImg);
            }
        }

        animations[animName] = Animation(tex, texFlipped, fWidth, tInterval, nFrames, type);
        if (currentAnimName.empty()) currentAnimName = animName;
    }

    void playAnimation(const std::string& animName) {
        if (currentAnimName == animName) return;
        if (animations.find(animName) == animations.end()) return;
        currentAnimName = animName;
        animTimer = 0;
        currentFrameIndex = 0;
        animDirection = 1;
        animFinished = false;
    }

    void updateSprite() {
        if (currentAnimName.empty() ||
            animations.find(currentAnimName) == animations.end()) return;

        Animation& anim = animations[currentAnimName];
        animTimer++;
        if (animTimer >= anim.timeInterval) {
            animTimer = 0;
            currentFrameIndex += animDirection;

            if (anim.type == ANIM_LOOP) {
                if (currentFrameIndex >= anim.numFrames) currentFrameIndex = 0;
            }
            else if (anim.type == ANIM_ONCE) {
                if (currentFrameIndex >= anim.numFrames) {
                    currentFrameIndex = anim.numFrames - 1;
                    animFinished = true;
                }
            }
            else if (anim.type == ANIM_PINGPONG) {
                if (currentFrameIndex >= anim.numFrames) {
                    animDirection = -1;
                    currentFrameIndex = anim.numFrames - 2;
                    if (currentFrameIndex < 0) currentFrameIndex = 0;
                }
                else if (currentFrameIndex < 0) {
                    animDirection = 1;
                    currentFrameIndex = 1;
                    if (currentFrameIndex >= anim.numFrames) currentFrameIndex = 0;
                }
            }
        }
    }

    // ============================================================
    // RENDER
    // ============================================================
    Rectangle getCurrentSrcRect() const {
        if (animations.find(currentAnimName) == animations.end())
            return { 0, 0, (float)visualW, (float)visualH };

        const Animation& anim = animations.at(currentAnimName);
        float fw = (float)anim.frameWidth;
        float fh = (float)anim.texture.height;
        float sx = (float)(anim.frameWidth * currentFrameIndex);
        return { sx, 0.0f, fw, fh };
    }

    // ============================================================
    // draw y ancla (desde abajo al centro))
    // ============================================================
    void draw(int scrollX, int scrollY) const {
        if (isHidden) return;
        if (animations.find(currentAnimName) == animations.end()) return;

        if (x - scrollX + w < 0 || x - scrollX > GetScreenWidth() ||
            y - scrollY + h < 0 || y - scrollY > GetScreenHeight()) return;

        int drawX = (int)(x + (w / 2.0f)) - (visualW / 2) - scrollX;
        int drawY = (int)(y + h) - visualH - scrollY;

        bool drawMainSprite = true;

        if (isInvincible && !isTransforming) {
            if (currentInvincibilityFrames % 4 < 2) drawMainSprite = false;
        }

        if (isStarInvincible) {
            int flashSpeed = (starTimer < 210) ? 12 : 3;
            int auraPhase = (starTimer / flashSpeed) % 3;
            Color auraColor = (auraPhase == 0) ? YELLOW : (auraPhase == 1) ? GREEN : RED;
            DrawCircle(drawX + (visualW / 2), drawY + visualH - h, (float)(w) * 0.75f, auraColor);
            if ((starTimer / flashSpeed) % 2 != 0) drawMainSprite = false;
        }

        if (!drawMainSprite) return;

        const Animation& anim = animations.at(currentAnimName);
        if (!anim.isValid()) return;

        // Elegir textura segun direccion — sin operaciones de flip en draw
        const Texture2D& tex = (lookingLeft && anim.textureFlipped.id != 0)
            ? anim.textureFlipped
            : anim.texture;

        Rectangle src = getCurrentSrcRect();
        Rectangle dst = {
            (float)drawX,
            (float)drawY,
            (float)anim.frameWidth,
            (float)anim.texture.height
        };
        DrawTexturePro(tex, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
    }

    // ============================================================
    // draw_old con ancla desde arriba izquierda (DEPRECIADO)
    // ============================================================
    void draw_old(int scrollX, int scrollY) const {
        if (isHidden) return;
        if (animations.find(currentAnimName) == animations.end()) return;

        if (x - scrollX + w < 0 || x - scrollX > GetScreenWidth() ||
            y - scrollY + h < 0 || y - scrollY > GetScreenHeight()) return;

        const Animation& anim = animations.at(currentAnimName);
        if (!anim.isValid()) return;

        const Texture2D& tex = (lookingLeft && anim.textureFlipped.id != 0)
            ? anim.textureFlipped
            : anim.texture;

        Rectangle src = getCurrentSrcRect();
        Rectangle dst = {
            (float)((int)x - scrollX),
            (float)((int)y - scrollY),
            (float)anim.frameWidth,
            (float)anim.texture.height
        };
        DrawTexturePro(tex, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
    }

    // ============================================================
    // DEBUG
    // ============================================================
    void debugInfo(int textX, int textY) const {
        DrawText(TextFormat("%s: x=%.1f y=%.1f", name.c_str(), x, y),
            textX, textY, 10, WHITE);
        DrawText(TextFormat("vx=%.1f vy=%.1f", vx, vy),
            textX, textY + 12, 10, WHITE);
        DrawText(TextFormat("Grounded: %d", (int)isGrounded),
            textX, textY + 24, 10, WHITE);
        DrawText(TextFormat("Vidas: %d", vidas),
            textX, textY + 36, 10, WHITE);
        DrawText(TextFormat("isPush?: %d", (int)isPushing),
            textX, textY + 48, 10, WHITE);
        DrawText(TextFormat("turboRunning?: %d", (int)turboRunning),
            textX, textY + 60, 10, WHITE);
        DrawText(TextFormat("turboHoldCount: %d", turboHoldCount),
            textX, textY + 72, 10, WHITE);
    }

    // Dibuja el collider como rectángulo de debug
    void debugDrawCollider(int scrollX, int scrollY, Color color) const {
        if (colliderType == COLLIDER_SPHERE) {
            DrawCircleLines(
                (int)(sphere.cx - scrollX),
                (int)(sphere.cy - scrollY),
                sphere.radius,
                color
            );
        }
        else {
            DrawRectangleLines(
                (int)collider.x - scrollX,
                (int)collider.y - scrollY,
                collider.w,
                collider.h,
                color
            );
        }
    }
};

#endif // ACTOR_H