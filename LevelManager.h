#ifndef LEVELMANAGER_H
#define LEVELMANAGER_H

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include "raylib.h"          // <-- Raylib en lugar de allegro.h
#include "ActorManager.h"
#include "TileMap.h"
#include "AIController.h"
#include "PlatformController.h"
#include "ActorFactory.h"

class LevelManager {
public:
    Actor* p1;
    Actor* p2;
    std::string backgroundPath;
    int mapWidth;
    int mapHeight;

    // Cache de texturas: BITMAP* -> Texture2D
    std::map<std::string, Texture2D> levelSprites;
    std::map<char, std::string> actorLegend;

    LevelManager() : p1(NULL), p2(NULL), mapWidth(0), mapHeight(0) {}

    ~LevelManager() {
        for (std::map<std::string, Texture2D>::iterator it = levelSprites.begin(); it != levelSprites.end(); ++it) {
            UnloadTexture(it->second);   // destroy_bitmap -> UnloadTexture
        }
        levelSprites.clear();
        actorLegend.clear();
    }

    void clear() {
        for (std::map<std::string, Texture2D>::iterator it = levelSprites.begin(); it != levelSprites.end(); ++it) {
            UnloadTexture(it->second);   // destroy_bitmap -> UnloadTexture
        }
        levelSprites.clear();
        actorLegend.clear();
        p1 = NULL;
        p2 = NULL;
    }

    bool loadLevel(const std::string& filepath,
        ActorManager& actors,
        TileMap& map,
        std::vector<AIController>& aiList,
        std::vector<PlatformController>& platList) {

        std::ifstream file(filepath.c_str());
        if (!file.is_open()) return false;

        std::string line;
        int section = 0; // 0=Ninguna, 1=Config, 2=Tiles, 3=Tilemap, 4=Actors, 5=Sprites, 6=ActorLegend
        int row = 0;

        while (std::getline(file, line)) {
            if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1);
            if (line.empty() || line[0] == '#') continue;

            // Detectar secciones
            if (line == "[CONFIG]") { section = 1; continue; }
            if (line == "[TILES]") { section = 2; continue; }
            if (line == "[TILEMAP]") {
                section = 3;
                row = 0;
                map.init(mapWidth, mapHeight, 32);
                continue;
            }
            if (line == "[ACTORS]") { section = 4; continue; }
            if (line == "[SPRITES]") { section = 5; continue; }
            if (line == "[ACTOR_LEGEND]") { section = 6; continue; }

            // ================================
            // LECTURA DE [CONFIG]
            // ================================
            if (section == 1) {
                std::istringstream iss(line);
                std::string key, val;
                if (iss >> key >> val) {
                    if (key == "width")      mapWidth = atoi(val.c_str());
                    else if (key == "height")     mapHeight = atoi(val.c_str());
                    else if (key == "background") backgroundPath = val;
                }
            }
            // ================================
            // LECTURA DE [TILES]
            // ================================
            else if (section == 2) {
                std::istringstream iss(line);
                int id;
                std::string path;
                if (iss >> id >> path) {
                    map.setTileImage(id, path.c_str());
                }
            }
            // ================================
            // LECTURA DE GRILLA [TILEMAP]
            // ================================
            else if (section == 3) {
                for (size_t col = 0; col < line.size(); col++) {
                    char c = line[col];

                    if (c == '0') {
                        map.setTile(col, row, 0);
                    }
                    else if (c >= '1' && c <= '9') {
                        map.setTile(col, row, c - '0');
                    }
                    else if (isalpha(c)) {
                        map.setTile(col, row, 0); // Aire detras del actor

                        if (actorLegend.find(c) != actorLegend.end()) {
                            std::string actorType = actorLegend[c];
                            float spawnX = col * 32.0f;
                            float spawnY = row * 32.0f;

                            Actor* newActor = ActorFactory::spawn(actorType, spawnX, spawnY, actors, aiList, platList, levelSprites);

                            if (newActor != NULL) {
                                if (actorType == "PLAYER1" || actorType == "PLAYER_MARIO") p1 = newActor;
                                else if (actorType == "PLAYER2") p2 = newActor;
                            }
                        }
                    }
                }
                row++;
            }
            // ================================
            // LECTURA DE [ACTORS]
            // ================================
            else if (section == 4) {
                std::istringstream iss(line);
                std::string type;
                float x, y;

                if (iss >> type >> x >> y) {
                    Actor* newActor = ActorFactory::spawn(type, x, y, actors, aiList, platList, levelSprites);

                    if (newActor != NULL) {
                        if (type == "PLAYER1" || type == "PLAYER_MARIO") p1 = newActor;
                        else if (type == "PLAYER2") p2 = newActor;
                    }
                }
            }
            // ================================
            // LECTURA DE [SPRITES] (Cache local)
            // ================================
            else if (section == 5) {
                std::istringstream iss(line);
                std::string spriteName, path;
                if (iss >> spriteName >> path) {
                    // load_bitmap -> LoadTexture
                    levelSprites[spriteName] = LoadTexture(path.c_str());
                }
            }
            // ================================
            // LECTURA DE [ACTOR_LEGEND]
            // ================================
            else if (section == 6) {
                std::istringstream iss(line);
                char symbol;
                std::string actorType;

                if (iss >> symbol >> actorType) {
                    actorLegend[symbol] = actorType;
                }
            }
        }
        return true;
    }
};

#endif // LEVELMANAGER_H