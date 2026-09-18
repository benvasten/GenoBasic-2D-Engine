#ifndef GAMESCREENS_H
#define GAMESCREENS_H

#include <raylib.h>

// ============================================================
//  GameScreens
//  Este struct re-renderiza las pantallas estáticas
//  (menú, win, gameover, credits) y las dibuja bajo demanda.
// ============================================================

enum ScreenID {
    SCREEN_MENU,
    SCREEN_WIN,
    SCREEN_GAMEOVER,
    SCREEN_GAMEBEATEN,
    SCREEN_COUNT
};

struct GameScreens {

    RenderTexture2D textures[SCREEN_COUNT];
    int w, h;

    //  Helper para texto centrado

    void _drawCenteredText(const char* text, int y, int fontSize, Color color) {
        int textW = MeasureText(text, fontSize);
        DrawText(text, w/2 - textW/2, y, fontSize, color);
    }

    GameScreens(int screenW, int screenH) : w(screenW), h(screenH) {
        for (int i = 0; i < SCREEN_COUNT; i++) textures[i] = { 0 };

        const int FS  = 20; // font size principal
        const int FSS = 14; // font size créditos
        Color white = WHITE;

        // ---- MENU ----
        textures[SCREEN_MENU] = LoadRenderTexture(w, h);
        BeginTextureMode(textures[SCREEN_MENU]);
            ClearBackground({ 20, 20, 100, 255 });
            _drawCenteredText("MI JUEGO - PRESIONA ENTER PARA JUGAR", h/2, FS, white);
        EndTextureMode();

        // ---- WIN ----
        textures[SCREEN_WIN] = LoadRenderTexture(w, h);
        BeginTextureMode(textures[SCREEN_WIN]);
            ClearBackground({ 20, 100, 20, 255 });
            _drawCenteredText("NIVEL SUPERADO! - PRESIONA ENTER", h/2, FS, white);
        EndTextureMode();

        // ---- GAME OVER ----
        textures[SCREEN_GAMEOVER] = LoadRenderTexture(w, h);
        BeginTextureMode(textures[SCREEN_GAMEOVER]);
            ClearBackground({ 100, 20, 20, 255 });
            _drawCenteredText("HAS MUERTO! - PRESIONA ENTER", h/2, FS, white);
        EndTextureMode();

        // ---- GAME BEATEN / CRÉDITOS ----
        textures[SCREEN_GAMEBEATEN] = LoadRenderTexture(w, h);
        BeginTextureMode(textures[SCREEN_GAMEBEATEN]);
            ClearBackground({ 150, 20, 100, 255 });
            _drawCenteredText("FELICIDADES, GRACIAS POR JUGAR", h/2,    FS,  white);
            _drawCenteredText("Diseno:       Tu Nombre",        h/2+30, FSS, white);
            _drawCenteredText("Programacion: Tu Nombre",        h/2+50, FSS, white);
            _drawCenteredText("Arte:         Tu Nombre",        h/2+70, FSS, white);
            _drawCenteredText("Testing:      Tu Nombre",        h/2+90, FSS, white);
            _drawCenteredText("Marketing:    Tu Nombre",        h/2+110,FSS, white);
        EndTextureMode();
    }

    void draw(ScreenID id) {
        if (textures[id].id == 0) return;
        Rectangle src = { 0, 0, (float)w, -(float)h };
        DrawTextureRec(textures[id].texture, src, { 0, 0 }, WHITE);
    }

    void destroy() {
        for (int i = 0; i < SCREEN_COUNT; i++) {
            if (textures[i].id != 0) {
                UnloadRenderTexture(textures[i]);
                textures[i] = { 0 };
            }
        }
    }
};

#endif // GAMESCREENS_H
