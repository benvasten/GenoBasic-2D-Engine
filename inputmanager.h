#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <raylib.h>

struct button {
    bool  down;
    bool  pressed;
    bool  released;
    bool  safe;
    float axis;
    bool  _prevDown;

    button() : down(false), pressed(false), released(false),
        safe(false), axis(0.0f), _prevDown(false) {}
};

struct stickAxis {
    float  x = 0.0f;
    float  y = 0.0f;
    button up, down, left, right;
};

struct inputs {
    button up, down, left, right;
    stickAxis leftStick;
    stickAxis rightStick;

    button btnA;   // Sur   -- Z / Numpad3 / A gamepad
    button btnB;   // Este  -- X / Numpad6 / B gamepad
    button btnX;   // Oeste -- S / Numpad5 / X gamepad
    button btnY;   // Norte -- A / Numpad2 / Y gamepad

    button lb;     // W    / Numpad4 / LB gamepad
    button rb;     // D    / Numpad8 / RB gamepad
    button lt;     // Q    / Numpad1 / LT gamepad (analogico en gamepad)
    button rt;     // C    / Numpad9 / RT gamepad (analogico en gamepad)

    button l3;     // E         / PageDown / L3 gamepad
    button r3;     // F         / PageUp   / R3 gamepad

    button start;  // CapsLock  / NumpadEnter / Start gamepad
    button select; // LShift    / NumpadPlus  / Back  gamepad

    button f3, f4, alt, enter;
};

enum class InputSource {
    KEYBOARD_P1,
    KEYBOARD_P2,
    GAMEPAD_0,
    GAMEPAD_1,
    GAMEPAD_2,
    GAMEPAD_3
};

struct playerConfig {
    InputSource source = InputSource::KEYBOARD_P1;
    int         gamepadId = 0;
    InputSource homeKeyboard = InputSource::KEYBOARD_P1; // fijo de por vida

    playerConfig() = default;
    explicit playerConfig(InputSource s)
        : source(s),
        gamepadId(s == InputSource::GAMEPAD_0 ? 0 :
            s == InputSource::GAMEPAD_1 ? 1 :
            s == InputSource::GAMEPAD_2 ? 2 :
            s == InputSource::GAMEPAD_3 ? 3 : 0),
        homeKeyboard(s == InputSource::KEYBOARD_P2
            ? InputSource::KEYBOARD_P2
            : InputSource::KEYBOARD_P1) {}
};

// ============================================================
//  PlayerInputManager
// ============================================================
struct PlayerInputManager {
    inputs       btn;
    playerConfig cfg;

    explicit PlayerInputManager(InputSource src = InputSource::KEYBOARD_P1)
        : cfg(src) {}

    // Llamar UNA VEZ POR FRAME por jugador, antes del update de juego.
    // Pasa el OTRO jugador para evitar conflictos de gamepad.
    void update(PlayerInputManager& other);

    // Nombre legible de la fuente activa (para debug / UI).
    const char* getSourceName() const;
};

// ============================================================
//  API bajo nivel (por si queremos acceso directo)
// ============================================================
void updateButton(button& b, bool isDownNow);
void updateButtonAnalog(button& b, bool isDownNow, float axisValue);
void keyPressesP1(inputs& p);
void keyPressesP2(inputs& p);
void gamepadPresses(inputs& p, int gamepadId);
void inputDetection(inputs& p, const playerConfig& cfg);
void inputDebug(inputs& p, int x, int y, bool showReleases = false);
bool anyButtonPressed(const inputs& p);
void clearEdges(inputs& p);

#endif // INPUTMANAGER_H