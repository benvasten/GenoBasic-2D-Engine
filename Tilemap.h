#ifndef TILEMAP_H
#define TILEMAP_H

#include "raylib.h"
#include <vector>
#include <cstring>   // strlen

#define TILE_SIZE 32

// ============================================================
// NOTA DE PROPIEDAD DE TEXTURAS
//   TileMap SÍ es dueño de sus Texture2D (a diferencia de Actor).
//   Las carga con setTileImage() / addTileImage() y las
//   descarga en el destructor con UnloadTexture().
//
//   Razón: las texturas de tiles son únicas por mapa y se
//   cargan desde rutas de archivo dentro del propio TileMap,
//   por lo que no hay cache externo que las gestione.
// ============================================================

class TileMap {
public:
    int width;     // Ancho en tiles
    int height;    // Alto en tiles
    int tileSize;

    // tiles[y * width + x] = tipo de tile (0 = aire, 1+ = sólido)
    std::vector<int> tiles;

    // tileTextures[tileID] = textura del tile
    // tileID == 0 → aire/vacío, su textura nunca se dibuja
    // Texture2D con id == 0 significa "no cargada"
    std::vector<Texture2D> tileTextures;


    // Constructores

    // Constructor vacío: LevelManager lo llama antes de conocer el tamaño
    TileMap() : width(0), height(0), tileSize(0) {}

    // Constructor con tamaño conocido desde el inicio
    TileMap(int w, int h, int tSize)
        : width(w), height(h), tileSize(tSize)
    {
        tiles.resize(width * height, 0);
    }

    // Destructor
    //   Descargar TODAS las texturas de la lista.
    //   Se llama al destruir el TileMap (fin de nivel o cierre).
    ~TileMap() {
        unloadAllTextures();
    }

    
    // init: redimensionar el mapa en caliente
    //   Usado por LevelManager cuando lee [CONFIG] y luego [TILEMAP].
    //   Descarga texturas existentes para evitar leaks al recargar niveles.
    
    void init(int w, int h, int tSize) {
        // Si ya había texturas cargadas (recarga de nivel), las liberamos
        //unloadAllTextures();

        width = w;
        height = h;
        tileSize = tSize;

        tiles.clear();
        tiles.assign(width * height, 0);
    }

    
    // addTileImage: añade una textura al final de la lista
    //   (equivale al addTileImage original con load_bitmap)
    
    void addTileImage(const char* spriteRoute) {
        Texture2D tex = LoadTexture(spriteRoute);
        tileTextures.push_back(tex);
    }

    
    // setTileImage: asigna una textura a un ID específico
    //   (equivale al setTileImage original con load_bitmap + destroy_bitmap)
    //
    //   CAMBIO vs Allegro:
    //     Antes: tileImages[id] = load_bitmap(path, NULL)
    //            if (tileImages[id] != NULL) destroy_bitmap(...)
    //     Ahora: if (tex.id != 0) UnloadTexture(tex)  ← id==0 es "vacía"
    //            tileTextures[id] = LoadTexture(path)
    
    void setTileImage(int tileID, const char* spriteRoute) {
        // Agrandar el vector si el ID pedido supera el tamaño actual
        if (tileID >= (int)tileTextures.size()) {
            Texture2D empty = { 0 };
            tileTextures.resize(tileID + 1, empty);
        }

        // Si ya había una textura válida en ese slot, la descargamos
        if (tileTextures[tileID].id != 0) {
            UnloadTexture(tileTextures[tileID]);
        }

        // Cargamos la nueva textura en el slot correcto
        tileTextures[tileID] = LoadTexture(spriteRoute);
    }

    
    // getTile / isSolid / setTile
    //   Sin cambios lógicos — solo quitamos la dependencia de Allegro
    

    int getTile(int x, int y) const {
        if (x < 0 || x >= width)  return 0;  // fronteras abiertas
        if (y < 0 || y >= height) return 0;
        return tiles[y * width + x];
    }

    bool isSolid(int x, int y) const {
        return getTile(x, y) >= 1;
    }

    void setTile(int x, int y, int value) {
        if (x >= 0 && y >= 0 && x < width && y < height) {
            tiles[y * width + x] = value;
        }
    }

    
    // Herramientas de construcción de niveles
    //   Sin cambios: solo operan sobre el array de ints
    

    void drawHLine(int startX, int endX, int y, int tileID) {
        for (int x = startX; x <= endX; x++) setTile(x, y, tileID);
    }

    void drawVLine(int x, int startY, int endY, int tileID) {
        for (int y = startY; y <= endY; y++) setTile(x, y, tileID);
    }

    void fillRegion(int startX, int startY, int endX, int endY, int tileID) {
        for (int y = startY; y <= endY; y++)
            for (int x = startX; x <= endX; x++)
                setTile(x, y, tileID);
    }

    
    // draw: render con culling (solo tiles en pantalla)
    
    void draw(int scrollX, int scrollY) const {
        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();

        // Calcular qué columnas y filas de tiles son visibles
        int startCol = scrollX / tileSize;
        int startRow = scrollY / tileSize;
        int endCol = (scrollX + screenW) / tileSize + 1;
        int endRow = (scrollY + screenH) / tileSize + 1;

        // Clamping a los límites del mapa
        if (startCol < 0)      startCol = 0;
        if (startRow < 0)      startRow = 0;
        if (endCol > width)  endCol = width;
        if (endRow > height) endRow = height;

        for (int row = startRow; row < endRow; row++) {
            for (int col = startCol; col < endCol; col++) {

                int tileID = getTile(col, row);

                // tileID == 0 es aire → no dibujar
                if (tileID <= 0) continue;

                // Verificar que el ID tiene textura asignada
                if (tileID >= (int)tileTextures.size()) continue;
                const Texture2D& tex = tileTextures[tileID];
                if (tex.id == 0) continue;   // textura no cargada

                int drawX = col * tileSize - scrollX;
                int drawY = row * tileSize - scrollY;

                DrawTexture(tex, drawX, drawY, WHITE);
            }
        }
    }

    
    // drawDebugGrid: superpone la rejilla de colisiones (debug)
    
    void drawDebugGrid(int scrollX, int scrollY) const {
        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();

        int startCol = scrollX / tileSize;
        int startRow = scrollY / tileSize;
        int endCol = (scrollX + screenW) / tileSize + 1;
        int endRow = (scrollY + screenH) / tileSize + 1;

        if (startCol < 0)     startCol = 0;
        if (startRow < 0)     startRow = 0;
        if (endCol > width)   endCol = width;
        if (endRow > height)  endRow = height;

        for (int row = startRow; row < endRow; row++) {
            for (int col = startCol; col < endCol; col++) {
                int drawX = col * tileSize - scrollX;
                int drawY = row * tileSize - scrollY;

                if (isSolid(col, row)) {
                    // Sólido: rectángulo rojo semitransparente
                    DrawRectangleLines(drawX, drawY, tileSize, tileSize,
                        { 255, 0, 0, 120 });
                }
                else {
                    // Aire: línea gris muy tenue (solo para ver la rejilla)
                    DrawRectangleLines(drawX, drawY, tileSize, tileSize,
                        { 255, 255, 255, 20 });
                }
            }
        }
    }

private:
    // Liberar todas las texturas del vector (llamado en ~TileMap e init)
    void unloadAllTextures() {
        for (size_t i = 0; i < tileTextures.size(); i++) {
            if (tileTextures[i].id != 0) {
                UnloadTexture(tileTextures[i]);
            }
        }
        tileTextures.clear();
    }
};

#endif // TILEMAP_H