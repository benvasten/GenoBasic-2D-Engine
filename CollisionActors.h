#ifndef COLLISIONACTORS_H
#define COLLISIONACTORS_H

// ============================================================
// COLLISIONACTORS.H
// ============================================================

#include "Actor.h"
#include "TileMap.h"


// ============================================================
// HELPER: Overlap AABB (compatibilidad + uso interno)
// ============================================================

bool checkAABB(float x1, float y1, int w1, int h1,
    float x2, float y2, int w2, int h2)
{
    return checkAABBOverlap((float)x1, (float)y1, (float)w1, (float)h1,
        (float)x2, (float)y2, (float)w2, (float)h2);
}


// ============================================================
// Dispatch: overlap entre actores (posición actual)
// ============================================================

bool checkActorOverlap(const Actor* a, const Actor* b)
{
    bool aSphere = (a->colliderType == COLLIDER_SPHERE);
    bool bSphere = (b->colliderType == COLLIDER_SPHERE);

    if (!aSphere && !bSphere) {
        return checkAABB(a->collider.x, a->collider.y, a->collider.w, a->collider.h,
            b->collider.x, b->collider.y, b->collider.w, b->collider.h);
    }
    if (aSphere && bSphere) {
        return checkSphereVsSphere(a->sphere.cx, a->sphere.cy, a->sphere.radius,
            b->sphere.cx, b->sphere.cy, b->sphere.radius);
    }
    if (aSphere) {
        return checkSphereVsAABB(a->sphere.cx, a->sphere.cy, a->sphere.radius,
            b->collider.x, b->collider.y, (float)b->collider.w, (float)b->collider.h);
    }
    return checkSphereVsAABB(b->sphere.cx, b->sphere.cy, b->sphere.radius,
        a->collider.x, a->collider.y, (float)a->collider.w, (float)a->collider.h);
}


// ============================================================
// Dispatch: overlap en el frame anterior (regla de oro)
// ============================================================

bool checkActorOverlapPrev(const Actor* a, const Actor* b)
{
    bool aSphere = (a->colliderType == COLLIDER_SPHERE);
    bool bSphere = (b->colliderType == COLLIDER_SPHERE);

    if (!aSphere && !bSphere) {
        return checkAABB(a->prevX, a->prevY, a->collider.w, a->collider.h,
            b->prevX, b->prevY, b->collider.w, b->collider.h);
    }
    if (aSphere && bSphere) {
        return checkSphereVsSphere(a->sphere.prevCx, a->sphere.prevCy, a->sphere.radius,
            b->sphere.prevCx, b->sphere.prevCy, b->sphere.radius);
    }
    if (aSphere) {
        float prevBx = b->prevX;
        float prevBy = b->prevY;
        return checkSphereVsAABB(a->sphere.prevCx, a->sphere.prevCy, a->sphere.radius,
            prevBx, prevBy, (float)b->collider.w, (float)b->collider.h);
    }
    float prevAx = a->prevX;
    float prevAy = a->prevY;
    return checkSphereVsAABB(b->sphere.prevCx, b->sphere.prevCy, b->sphere.radius,
        prevAx, prevAy, (float)a->collider.w, (float)a->collider.h);
}


// ============================================================
// Dispatch: overlap con actor A en posición hipotética
// ============================================================

bool checkActorOverlapAt(const Actor* a, float testX, float testY, const Actor* b)
{
    bool aSphere = (a->colliderType == COLLIDER_SPHERE);
    bool bSphere = (b->colliderType == COLLIDER_SPHERE);

    if (!aSphere && !bSphere) {
        return checkAABB(testX, testY, a->w, a->h,
            b->collider.x, b->collider.y, b->collider.w, b->collider.h);
    }
    if (aSphere && bSphere) {
        float cx = testX + a->sphereOffsetX;
        float cy = testY + a->sphereOffsetY;
        return checkSphereVsSphere(cx, cy, a->sphere.radius,
            b->sphere.cx, b->sphere.cy, b->sphere.radius);
    }
    if (aSphere) {
        float cx = testX + a->sphereOffsetX;
        float cy = testY + a->sphereOffsetY;
        return checkSphereVsAABB(cx, cy, a->sphere.radius,
            b->collider.x, b->collider.y, (float)b->collider.w, (float)b->collider.h);
    }
    return checkSphereVsAABB(b->sphere.cx, b->sphere.cy, b->sphere.radius,
        testX, testY, (float)a->w, (float)a->h);
}


// ============================================================
// HELPER: ¿Puede el actor moverse a (testX, testY)?
// ============================================================

bool canMoveToPosition(Actor* actor, float testX, float testY,
    const TileMap& tilemap, Actor* actors[], int count,
    Actor* exclude = NULL)
{
    float testLeft, testTop;
    int testW, testH;

    if (actor->colliderType == COLLIDER_SPHERE) {
        float cx = testX + actor->sphereOffsetX;
        float cy = testY + actor->sphereOffsetY;
        sphereToAABB(cx, cy, actor->sphere.radius, testLeft, testTop, testW, testH);
    }
    else {
        testLeft = testX;
        testTop = testY;
        testW = actor->collider.w;
        testH = actor->collider.h;
    }

    int tileLeft = (int)floor(testLeft / TILE_SIZE);
    int tileRight = (int)floor((testLeft + testW - 1) / TILE_SIZE);
    int tileTop = (int)floor(testTop / TILE_SIZE);
    int tileBottom = (int)floor((testTop + testH - 1) / TILE_SIZE);

    for (int ty = tileTop; ty <= tileBottom; ty++)
        for (int tx = tileLeft; tx <= tileRight; tx++)
            if (tilemap.isSolid(tx, ty)) return false;

    for (int i = 0; i < count; i++) {
        Actor* other = actors[i];

        if (actor == other)              continue;
        if (!other->alive || !other->isSolid) continue;
        if (exclude != NULL && other == exclude) continue;

        if (checkActorOverlapPrev(actor, other)) continue;

        bool blocked = false;
        if (actor->colliderType == COLLIDER_SPHERE) {
            float cx = testX + actor->sphereOffsetX;
            float cy = testY + actor->sphereOffsetY;
            if (other->colliderType == COLLIDER_SPHERE) {
                blocked = checkSphereVsSphere(cx, cy, actor->sphere.radius,
                    other->sphere.cx, other->sphere.cy, other->sphere.radius);
            }
            else {
                blocked = checkSphereVsAABB(cx, cy, actor->sphere.radius,
                    other->collider.x, other->collider.y,
                    (float)other->collider.w, (float)other->collider.h);
            }
        }
        else if (other->colliderType == COLLIDER_SPHERE) {
            blocked = checkSphereVsAABB(other->sphere.cx, other->sphere.cy, other->sphere.radius,
                testX, testY, (float)actor->w, (float)actor->h);
        }
        else {
            blocked = checkAABB(testX, testY, actor->w, actor->h,
                other->collider.x, other->collider.y,
                other->collider.w, other->collider.h);
        }
        if (blocked) return false;
    }

    return true;
}


// ============================================================
// HELPER: ¿Es seguro ponerse de pie?
// ============================================================

bool canStandUpSafe(Actor* actor, const TileMap& tilemap,
    Actor* actors[], int count)
{
    if (actor->colliderType == COLLIDER_SPHERE) return true;

    int tempH = actor->collider.h;
    actor->collider.h = actor->originalHeight;

    float testY = actor->y - (actor->originalHeight - actor->crouchHeight);
    bool  isSafe = canMoveToPosition(actor, actor->x, testY, tilemap, actors, count);

    actor->collider.h = tempH;
    return isSafe;
}


// ============================================================
// COLISIÓN ACTOR VS ACTOR — EJE X
// ============================================================

void resolveActorVsActorX(Actor* a, Actor* b,
    const TileMap& tilemap, Actor* actors[], int count,
    bool forceCollision = false)
{
    if (!a->alive || !b->alive) return;
    if (!forceCollision && (!a->isSolid || !b->isSolid)) return;

    if (a->isShell && (!b->isSolid || b->canPush)) return;
    if (b->isShell && (!a->isSolid || a->canPush)) return;

    if (checkActorOverlapPrev(a, b)) return;

    if (!checkActorOverlap(a, b)) return;

    float deltaA_X = a->collider.x - a->prevX;
    float deltaB_X = b->collider.x - b->prevX;
    float relativeVx = deltaA_X - deltaB_X;

    bool aCanPushB = a->canPush && b->isPushable && !b->canCarry;
    bool bCanPushA = b->canPush && a->isPushable && !a->canCarry;

    if (aCanPushB && bCanPushA && relativeVx != 0) {
        if (relativeVx > 0) { a->x = b->collider.x - a->collider.w; a->syncCollider(); }
        else { a->x = b->collider.x + b->collider.w; a->syncCollider(); }
        a->vx = 0;
        b->vx = 0;
        return;
    }

    if (aCanPushB && relativeVx != 0) {
        float targetX = b->collider.x + relativeVx;
        if (canMoveToPosition(b, targetX, b->collider.y, tilemap, actors, count, a)) {
            b->x = targetX;
            b->syncCollider();
            a->isPushing = true;
        }
        else {
            if (relativeVx > 0) { a->x = b->collider.x - a->collider.w; }
            else { a->x = b->collider.x + b->collider.w; }
            a->syncCollider();
            a->vx = 0;
        }
        return;
    }

    if (bCanPushA && relativeVx != 0) {
        float targetX = a->collider.x - relativeVx;
        if (canMoveToPosition(a, targetX, a->collider.y, tilemap, actors, count, b)) {
            a->x = targetX;
            a->syncCollider();
            b->isPushing = true;
        }
        else {
            if (relativeVx > 0) { b->x = a->collider.x + a->collider.w; }
            else { b->x = a->collider.x - b->collider.w; }
            b->syncCollider();
            b->vx = 0;
        }
        return;
    }

    if (relativeVx > 0) {
        a->x = b->collider.x - a->collider.w;
        a->syncCollider();
        a->vx = 0;
    }
    else if (relativeVx < 0) {
        a->x = b->collider.x + b->collider.w;
        a->syncCollider();
        a->vx = 0;
    }
}


// ============================================================
// COLISIÓN ACTOR VS ACTOR — EJE Y
// ============================================================

void resolveActorVsActorY(Actor* a, Actor* b, bool forceCollision = false)
{
    if (!a->alive || !b->alive) return;
    if (!forceCollision && (!a->isSolid || !b->isSolid)) return;

    if (checkActorOverlapPrev(a, b)) return;

    float nextY = a->y + a->vy;

    if (!checkActorOverlapAt(a, a->x, nextY, b)) return;

    if (a->vy > 0) {
        if (a->collider.y + a->collider.h <= b->collider.y + 2) {
            a->y = b->collider.y - a->collider.h;
            a->syncCollider();
            a->vy = 0;
            a->isGrounded = true;
            if (b->canCarry) a->carriedBy = b;
        }
    }
    else if (a->intendedVy < 0) {
        a->y = b->collider.y + b->collider.h;
        a->syncCollider();
        a->vy = 0;

        if (b->isHitableFromBelow && b->beenHitAsBlock == NOTHIT) {
            b->originalBlock_Y = b->y;
            b->beenHitAsBlock = BEINGHIT;
            float hitX = (a->colliderType == COLLIDER_SPHERE)
                ? a->sphere.cx
                : a->collider.x + (a->collider.w / 2.0f);
            b->lastHitByX = hitX;
            b->hitByBigMario = (a->animPrefix == "BIG_" || a->animPrefix == "FIRE_");
        }
        else if (b->isHitableFromBelow && b->beenHitAsBlock == ALREADYHIT) {
            a->bumpedHead = true;
        }
        else if (b->isSolid && !b->isHitableFromBelow) {
            a->bumpedHead = true;
        }
    }
}

#endif // COLLISIONACTORS_H
