#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <raylib.h>

// ============================================================
//  DisplayManager
// ============================================================

enum DisplayMode {
    DISPLAY_WINDOWED = 0,
    DISPLAY_BORDERLESS,
    DISPLAY_FULLSCREEN,
    DISPLAY_MODE_COUNT
};

struct DisplayManager {

    RenderTexture2D virtualRT;
    int             virtualW, virtualH;
    DisplayMode     currentMode;

    // Tamaño de la ventana al estar en modo WINDOWED
    // (se guarda al salir de ese modo para poder restaurarlo)
    int savedWindowW, savedWindowH;

    // Overlay de notificación de cambio de modo
    const char* notifyText;
    float       notifyTimer;       // segundos restantes
    static const float NOTIFY_DURATION; // definido abajo

    // ----------------------------------------------------------
    //  IMPORTANTE:
    // ----------------------------------------------------------
    //  NUNCA LLAMES InitWindow antes de esta función.
    //  Este init llama a InitWindow internamente
    //  startMode: modo inicial (por defecto WINDOWED a 2×)
    // ----------------------------------------------------------
    void init(int vW, int vH, const char* title,
        DisplayMode startMode = DISPLAY_WINDOWED) {

        virtualW = vW;
        virtualH = vH;

        // Ventana redimensionable
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);

        int monitor = 0; // monitor primario
        int screenW = GetMonitorWidth(monitor);
        int screenH = GetMonitorHeight(monitor);

        // calcular el mayor integer scale posible
        int scaleX = screenW / vW;
        int scaleY = screenH / vH;
        int scale = (scaleX < scaleY) ? scaleX : scaleY;

        if (scale > 2) scale = 2;   // limitar a 2x como en tu versión SDL
        if (scale < 1) scale = 1;

        savedWindowW = vW * scale;
        savedWindowH = vH * scale;

        InitWindow(savedWindowW, savedWindowH, title);

        // RenderTexture a resolución virtual
        virtualRT = LoadRenderTexture(vW, vH);
        SetTextureFilter(virtualRT.texture, TEXTURE_FILTER_POINT); // sin blur

        currentMode = DISPLAY_WINDOWED;
        notifyText = "";
        notifyTimer = 0.0f;

        // Aplicar modo inicial si no es WINDOWED
        if (startMode != DISPLAY_WINDOWED)
            setMode(startMode);
    }

    // ----------------------------------------------------------
    //  salida con ESCAPE
    // ----------------------------------------------------------
    bool shouldClose() const {
        return WindowShouldClose();
    }

    int getIntegerScale() const {
        int scaleX = GetScreenWidth() / virtualW;
        int scaleY = GetScreenHeight() / virtualH;
        int scale = (scaleX < scaleY) ? scaleX : scaleY;
        return (scale < 1) ? 1 : scale;
    }

    // ----------------------------------------------------------
    //  setMode: para cambiar el modo de pantalla en runtime
    // ----------------------------------------------------------
    void setMode(DisplayMode newMode) {

        // Guardar tamaño actual si estamos en ventana
        if (currentMode == DISPLAY_WINDOWED) {
            savedWindowW = GetScreenWidth();
            savedWindowH = GetScreenHeight();
        }

        // --- Salir del modo actual ---
        if (IsWindowFullscreen())
            ToggleFullscreen();

        ClearWindowState(FLAG_BORDERLESS_WINDOWED_MODE);

        // --- Entrar al nuevo modo ---
        currentMode = newMode;

        switch (newMode) {

        case DISPLAY_WINDOWED:
            SetWindowSize(savedWindowW, savedWindowH);
            // Centrar ventana en pantalla
            SetWindowPosition(
                (GetMonitorWidth(GetCurrentMonitor()) - savedWindowW) / 2,
                (GetMonitorHeight(GetCurrentMonitor()) - savedWindowH) / 2
            );
            notifyText = "Modo: Ventana";
            break;

        case DISPLAY_BORDERLESS:
            SetWindowState(FLAG_BORDERLESS_WINDOWED_MODE);
            notifyText = "Modo: Sin bordes";
            break;

        case DISPLAY_FULLSCREEN:
            // Usar la resolución nativa del monitor para fullscreen
            SetWindowSize(GetMonitorWidth(GetCurrentMonitor()),
                GetMonitorHeight(GetCurrentMonitor()));
            ToggleFullscreen();
            notifyText = "Modo: Pantalla completa";
            break;

        default: break;
        }

        notifyTimer = NOTIFY_DURATION;
    }

    // ----------------------------------------------------------
    //  cycleMode — F11: Ventana → Sin bordes → Fullscreen → Ventana
    // ----------------------------------------------------------
    void cycleMode() {
        setMode((DisplayMode)(((int)currentMode + 1) % DISPLAY_MODE_COUNT));
    }

    // ----------------------------------------------------------
    //  handleInput — detectar F11 para ciclar modos.
    //  Llamar UNA VEZ por frame, antes del loop de updates.
    // ----------------------------------------------------------
    void handleInput() {
        if (IsKeyPressed(KEY_F11)) cycleMode();
    }

    // ----------------------------------------------------------
    //  beginVirtualDraw
    //  Abre el BeginTextureMode sobre la RenderTexture virtual.
    //  Todo lo que se dibuje aquí queda en coordenadas 640×480.
    // ----------------------------------------------------------
    void beginVirtualDraw() {
        BeginTextureMode(virtualRT);
    }

    // ----------------------------------------------------------
    //  endVirtualDraw
    //  Cierra BeginTextureMode, escala con integer scaling al
    //  framebuffer real, y presenta el frame (BeginDrawing/EndDrawing).
    //
    //  El overlay de notificación se dibuja DESPUÉS del escalado,
    //  en coordenadas reales de pantalla (siempre legible).
    // ----------------------------------------------------------
    void endVirtualDraw() {
        EndTextureMode();

        // --- Calcular rectángulo destino centrado ---
        int   scale = getIntegerScale();
        float dstW = (float)(virtualW * scale);
        float dstH = (float)(virtualH * scale);
        float dstX = ((float)GetScreenWidth() - dstW) * 0.5f;
        float dstY = ((float)GetScreenHeight() - dstH) * 0.5f;

        // Fuente: altura negativa para corregir el flip Y de OpenGL
        Rectangle src = { 0.0f, 0.0f, (float)virtualW, -(float)virtualH };
        Rectangle dst = { dstX, dstY, dstW, dstH };

        BeginDrawing();
        ClearBackground(BLACK); // barras negras fuera del área escalada

        DrawTexturePro(virtualRT.texture, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);

        // --- Overlay de modo (en coordenadas reales) ---
        if (notifyTimer > 0.0f) {
            notifyTimer -= GetFrameTime();
            if (notifyTimer < 0.0f) notifyTimer = 0.0f;

            // Fade out en el último segundo
            float alpha = notifyTimer < 1.0f ? notifyTimer : 1.0f;
            unsigned char a = (unsigned char)(alpha * 220);

            // Fondo semitransparente para legibilidad
            DrawRectangle(8, 8, MeasureText(notifyText, 18) + 16, 30,
                { 0, 0, 0, (unsigned char)(a / 2) });
            DrawText(notifyText, 16, 14, 18, { 255, 255, 255, a });
        }

        EndDrawing();
    }

    // ----------------------------------------------------------
    //  currentScaleInfo — útil para mostrar en debug
    // ----------------------------------------------------------
    const char* getScaleInfo() const {
        int s = getIntegerScale();
        return TextFormat("Escala: %dx  (%dx%d)",
            s, virtualW * s, virtualH * s);
    }

    // ----------------------------------------------------------
    //  destroy — llamar antes de CloseWindow()
    // ----------------------------------------------------------
    void destroy() {
        UnloadRenderTexture(virtualRT);
    }
};

// Definición del static (necesaria en C++)
const float DisplayManager::NOTIFY_DURATION = 2.0f;

#endif // DISPLAYMANAGER_H#pragma once
