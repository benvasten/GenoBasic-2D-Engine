#include "inputmanager.h"
#include <raylib.h>

void updateButton(button& b, bool isDownNow) {
    b.pressed = (!b.down && isDownNow);
    b.released = (b.down && !isDownNow);
    b.down = isDownNow;
    b.axis = isDownNow ? 1.0f : 0.0f;
    b.safe = (isDownNow && !b._prevDown);
    b._prevDown = isDownNow;
}

void updateButtonAnalog(button& b, bool isDownNow, float axisValue) {
    b.pressed = (!b.down && isDownNow);
    b.released = (b.down && !isDownNow);
    b.down = isDownNow;
    b.axis = axisValue;
    b.safe = (isDownNow && !b._prevDown);
    b._prevDown = isDownNow;
}

static void updateStick(stickAxis& s, float x, float y, float dead = 0.5f) {
    s.x = x; s.y = y;
    updateButton(s.up, y < -dead);
    updateButton(s.down, y > dead);
    updateButton(s.left, x < -dead);
    updateButton(s.right, x > dead);
}

struct rawInputs {
    bool  up, down, left, right;
    float stickLX, stickLY;
    float stickRX, stickRY;
    bool  btnA, btnB, btnX, btnY;
    bool  lb, rb;
    bool  lt, rt;
    float ltAxis, rtAxis;
    bool  l3, r3;
    bool  start, select;
};

static rawInputs readKeyboardP1() {
    rawInputs r{};
    bool kUp = IsKeyDown(KEY_I), kDown = IsKeyDown(KEY_K);
    bool kLeft = IsKeyDown(KEY_J), kRight = IsKeyDown(KEY_L);
    r.up = kUp; r.down = kDown; r.left = kLeft; r.right = kRight;
    r.stickLX = kRight ? 1.0f : (kLeft ? -1.0f : 0.0f);
    r.stickLY = kDown ? 1.0f : (kUp ? -1.0f : 0.0f);
    r.stickRX = 0.0f; r.stickRY = 0.0f;
    r.btnA = IsKeyDown(KEY_A);      r.btnB = IsKeyDown(KEY_S);
    r.btnX = IsKeyDown(KEY_Q);      r.btnY = IsKeyDown(KEY_W);
    r.lb = IsKeyDown(KEY_TWO);      r.rb = IsKeyDown(KEY_E);
    r.lt = IsKeyDown(KEY_THREE);      r.ltAxis = r.lt ? 1.0f : 0.0f;
    r.rt = IsKeyDown(KEY_R);      r.rtAxis = r.rt ? 1.0f : 0.0f;
    r.l3 = IsKeyDown(KEY_FOUR);      r.r3 = IsKeyDown(KEY_T);
    r.start = IsKeyDown(KEY_CAPS_LOCK);
    r.select = IsKeyDown(KEY_LEFT_SHIFT);
    return r;
}

static rawInputs readKeyboardP2() {
    rawInputs r{};
    bool kUp = IsKeyDown(KEY_UP), kDown = IsKeyDown(KEY_DOWN);
    bool kLeft = IsKeyDown(KEY_LEFT), kRight = IsKeyDown(KEY_RIGHT);
    r.up = kUp; r.down = kDown; r.left = kLeft; r.right = kRight;
    r.stickLX = kRight ? 1.0f : (kLeft ? -1.0f : 0.0f);
    r.stickLY = kDown ? 1.0f : (kUp ? -1.0f : 0.0f);
    r.stickRX = 0.0f; r.stickRY = 0.0f;
    r.btnA = IsKeyDown(KEY_KP_3);   r.btnB = IsKeyDown(KEY_KP_6);
    r.btnX = IsKeyDown(KEY_KP_5);   r.btnY = IsKeyDown(KEY_KP_2);
    r.lb = IsKeyDown(KEY_KP_4);   r.rb = IsKeyDown(KEY_KP_8);
    r.lt = IsKeyDown(KEY_KP_1);   r.ltAxis = r.lt ? 1.0f : 0.0f;
    r.rt = IsKeyDown(KEY_KP_9);   r.rtAxis = r.rt ? 1.0f : 0.0f;
    r.l3 = IsKeyDown(KEY_PAGE_DOWN); r.r3 = IsKeyDown(KEY_PAGE_UP);
    r.start = IsKeyDown(KEY_KP_ENTER);
    r.select = IsKeyDown(KEY_KP_ADD);
    return r;
}

static rawInputs readGamepad(int id) {
    rawInputs r{};
    if (!IsGamepadAvailable(id)) return r;
    const float DEAD_TRIGGER = 0.3f;
    r.up = IsGamepadButtonDown(id, GAMEPAD_BUTTON_LEFT_FACE_UP);
    r.down = IsGamepadButtonDown(id, GAMEPAD_BUTTON_LEFT_FACE_DOWN);
    r.left = IsGamepadButtonDown(id, GAMEPAD_BUTTON_LEFT_FACE_LEFT);
    r.right = IsGamepadButtonDown(id, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
    float lx = GetGamepadAxisMovement(id, GAMEPAD_AXIS_LEFT_X);
    float ly = GetGamepadAxisMovement(id, GAMEPAD_AXIS_LEFT_Y);
    if (r.left)  lx = -1.0f; if (r.right) lx = 1.0f;
    if (r.up)    ly = -1.0f; if (r.down)  ly = 1.0f;
    r.stickLX = lx; r.stickLY = ly;
    r.stickRX = GetGamepadAxisMovement(id, GAMEPAD_AXIS_RIGHT_X);
    r.stickRY = GetGamepadAxisMovement(id, GAMEPAD_AXIS_RIGHT_Y);
    r.btnA = IsGamepadButtonDown(id, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
    r.btnB = IsGamepadButtonDown(id, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
    r.btnX = IsGamepadButtonDown(id, GAMEPAD_BUTTON_RIGHT_FACE_LEFT);
    r.btnY = IsGamepadButtonDown(id, GAMEPAD_BUTTON_RIGHT_FACE_UP);
    r.lb = IsGamepadButtonDown(id, GAMEPAD_BUTTON_LEFT_TRIGGER_1);
    r.rb = IsGamepadButtonDown(id, GAMEPAD_BUTTON_RIGHT_TRIGGER_1);
    float rawLT = GetGamepadAxisMovement(id, GAMEPAD_AXIS_LEFT_TRIGGER);
    float rawRT = GetGamepadAxisMovement(id, GAMEPAD_AXIS_RIGHT_TRIGGER);
    r.ltAxis = (rawLT + 1.0f) * 0.5f;
    r.rtAxis = (rawRT + 1.0f) * 0.5f;
    r.lt = r.ltAxis > DEAD_TRIGGER; r.rt = r.rtAxis > DEAD_TRIGGER;
    r.l3 = IsGamepadButtonDown(id, GAMEPAD_BUTTON_LEFT_THUMB);
    r.r3 = IsGamepadButtonDown(id, GAMEPAD_BUTTON_RIGHT_THUMB);
    r.start = IsGamepadButtonDown(id, GAMEPAD_BUTTON_MIDDLE_RIGHT);
    r.select = IsGamepadButtonDown(id, GAMEPAD_BUTTON_MIDDLE_LEFT);
    return r;
}

static void applyRaw(inputs& p, const rawInputs& r) {
    updateButton(p.up, r.up);     updateButton(p.down, r.down);
    updateButton(p.left, r.left); updateButton(p.right, r.right);
    updateStick(p.leftStick, r.stickLX, r.stickLY);
    updateStick(p.rightStick, r.stickRX, r.stickRY);
    updateButton(p.btnA, r.btnA); updateButton(p.btnB, r.btnB);
    updateButton(p.btnX, r.btnX); updateButton(p.btnY, r.btnY);
    updateButton(p.lb, r.lb);     updateButton(p.rb, r.rb);
    updateButtonAnalog(p.lt, r.lt, r.ltAxis);
    updateButtonAnalog(p.rt, r.rt, r.rtAxis);
    updateButton(p.l3, r.l3);     updateButton(p.r3, r.r3);
    updateButton(p.start, r.start); updateButton(p.select, r.select);
}

void gamepadPresses(inputs& p, int id) { applyRaw(p, readGamepad(id)); }
void keyPressesP1(inputs& p) {
    applyRaw(p, readKeyboardP1());
    updateButton(p.f3, IsKeyDown(KEY_F3));
    updateButton(p.f4, IsKeyDown(KEY_F4));
    updateButton(p.alt, IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT));
    updateButton(p.enter, IsKeyDown(KEY_ENTER));
}
void keyPressesP2(inputs& p) { applyRaw(p, readKeyboardP2()); }

void inputDetection(inputs& p, const playerConfig& cfg) {
    switch (cfg.source) {
    case InputSource::KEYBOARD_P1:
        applyRaw(p, readKeyboardP1());
        updateButton(p.f3, IsKeyDown(KEY_F3));
        updateButton(p.f4, IsKeyDown(KEY_F4));
        updateButton(p.alt, IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT));
        updateButton(p.enter, IsKeyDown(KEY_ENTER));
        break;
    case InputSource::KEYBOARD_P2:
        applyRaw(p, readKeyboardP2());
        break;
    case InputSource::GAMEPAD_0:
    case InputSource::GAMEPAD_1:
    case InputSource::GAMEPAD_2:
    case InputSource::GAMEPAD_3:
        applyRaw(p, readGamepad(cfg.gamepadId));
        break;
    }
}

void inputDebug(inputs& p, int x, int y, bool showReleases) {
    Color col = YELLOW;
    Color cyan = { 0, 255, 255, 255 };
    Color gray = LIGHTGRAY;
    int   fs = 10;
    DrawText(TextFormat("DP U:%d D:%d L:%d R:%d",
        p.up.down, p.down.down, p.left.down, p.right.down), x, y, fs, col);
    DrawText(TextFormat("LS x:%.2f y:%.2f", p.leftStick.x, p.leftStick.y), x, y + 14, fs, cyan);
    DrawText(TextFormat("LS U:%d D:%d L:%d R:%d",
        p.leftStick.up.down, p.leftStick.down.down,
        p.leftStick.left.down, p.leftStick.right.down), x, y + 26, fs, cyan);
    DrawText(TextFormat("RS x:%.2f y:%.2f", p.rightStick.x, p.rightStick.y), x, y + 40, fs, gray);
    DrawText(TextFormat("RS U:%d D:%d L:%d R:%d",
        p.rightStick.up.down, p.rightStick.down.down,
        p.rightStick.left.down, p.rightStick.right.down), x, y + 52, fs, gray);
    DrawText(TextFormat("A:%d B:%d X:%d Y:%d",
        p.btnA.down, p.btnB.down, p.btnX.down, p.btnY.down), x, y + 66, fs, col);
    DrawText(TextFormat("LB:%d RB:%d  LT:%.2f RT:%.2f",
        p.lb.down, p.rb.down, p.lt.axis, p.rt.axis), x, y + 80, fs, col);
    DrawText(TextFormat("L3:%d R3:%d", p.l3.down, p.r3.down), x, y + 94, fs, col);
    DrawText(TextFormat("STR:%d SEL:%d", p.start.down, p.select.down), x, y + 108, fs, col);
    if (showReleases) {
        DrawText(TextFormat("rel A:%d B:%d X:%d Y:%d",
            p.btnA.released, p.btnB.released,
            p.btnX.released, p.btnY.released), x, y + 122, fs, col);
        DrawText(TextFormat("rel DP U:%d D:%d L:%d R:%d",
            p.up.released, p.down.released,
            p.left.released, p.right.released), x, y + 134, fs, col);
        DrawText(TextFormat("rel LS U:%d D:%d L:%d R:%d",
            p.leftStick.up.released, p.leftStick.down.released,
            p.leftStick.left.released, p.leftStick.right.released), x, y + 146, fs, cyan);
    }
}

bool anyButtonPressed(const inputs& p) {
    return p.btnA.pressed || p.btnB.pressed || p.btnX.pressed || p.btnY.pressed
        || p.lb.pressed || p.rb.pressed || p.lt.pressed || p.rt.pressed
        || p.l3.pressed || p.r3.pressed
        || p.start.pressed || p.select.pressed;
}

static void clearEdge(button& b) { b.pressed = b.released = b.safe = false; }
void clearEdges(inputs& p) {
    clearEdge(p.up);    clearEdge(p.down);
    clearEdge(p.left);  clearEdge(p.right);
    clearEdge(p.leftStick.up);    clearEdge(p.leftStick.down);
    clearEdge(p.leftStick.left);  clearEdge(p.leftStick.right);
    clearEdge(p.rightStick.up);   clearEdge(p.rightStick.down);
    clearEdge(p.rightStick.left); clearEdge(p.rightStick.right);
    clearEdge(p.btnA);  clearEdge(p.btnB);
    clearEdge(p.btnX);  clearEdge(p.btnY);
    clearEdge(p.lb);    clearEdge(p.rb);
    clearEdge(p.lt);    clearEdge(p.rt);
    clearEdge(p.l3);    clearEdge(p.r3);
    clearEdge(p.start); clearEdge(p.select);
    clearEdge(p.f3);    clearEdge(p.f4);
    clearEdge(p.alt);   clearEdge(p.enter);
}

// ============================================================
//  PlayerInputManager::update
// ============================================================
static bool isGamepad(InputSource s) { return s >= InputSource::GAMEPAD_0; }

static InputSource sourceFromId(int id) {
    return static_cast<InputSource>(
        static_cast<int>(InputSource::GAMEPAD_0) + id);
}

static bool gamepadAnyPressed(int id) {
    if (!IsGamepadAvailable(id)) return false;
    for (int b = 0; b <= GAMEPAD_BUTTON_RIGHT_THUMB; b++)
        if (IsGamepadButtonPressed(id, b)) return true;
    return false;
}

static int findFreeGamepad(int usedA, int usedB) {
    for (int id = 0; id < 4; id++) {
        if (id == usedA || id == usedB) continue;
        if (gamepadAnyPressed(id)) return id;
    }
    return -1;
}

void PlayerInputManager::update(PlayerInputManager& other) {

    // 1. Fallback a teclado si el gamepad se desconecto
    if (isGamepad(cfg.source) && !IsGamepadAvailable(cfg.gamepadId)) {
        InputSource home = cfg.homeKeyboard;
        cfg = playerConfig(home);
        cfg.homeKeyboard = home;
    }

    // 2. Auto-claim: si estamos en teclado y alguien presiona un pad libre
    if (!isGamepad(cfg.source)) {
        int otherPad = isGamepad(other.cfg.source) ? other.cfg.gamepadId : -1;
        int myPad = isGamepad(cfg.source) ? cfg.gamepadId : -1;
        int claimed = findFreeGamepad(myPad, otherPad);
        if (claimed != -1) {
            InputSource home = cfg.homeKeyboard;
            cfg.source = sourceFromId(claimed);
            cfg.gamepadId = claimed;
            cfg.homeKeyboard = home;
        }
    }

    // 3. R3 en gamepad -> volver al teclado de origen (PAD->KB)
    if (isGamepad(cfg.source) && btn.r3.pressed) {
        InputSource home = cfg.homeKeyboard;
        cfg = playerConfig(home);
        cfg.homeKeyboard = home;
    }

    // 4. Leer hardware
    inputDetection(btn, cfg);
}

const char* PlayerInputManager::getSourceName() const {
    switch (cfg.source) {
    case InputSource::KEYBOARD_P1: return "KB-P1";
    case InputSource::KEYBOARD_P2: return "KB-P2";
    default: return TextFormat("PAD %d", cfg.gamepadId);
    }
}