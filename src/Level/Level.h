#ifndef LEVEL_H
#define LEVEL_H

#include "Tile.h"
#include "Spritesheet.h"
#include "Entity.h"

#include <vector>

class Level {
public:
    Level(int w, int h);
    ~Level() = default;

    void render(int xOffset, int yOffset);

    void fileTilemapWithDefaultLayout();
    void onTileMovedFrom(int x, int y, Entity* e, bool wasPushed);
    void onTileMovedTo(int x, int y, Entity* e, bool wasPushed);

    void setTile(int x, int y, Tile tile);
    void setLevelSpritesheet(Spritesheet* spritesheet);
    void setRenderPos(int x, int y);
    void addTileEvent(Entity* e, TileEvent::TileEvent event);

    int getWidth();
    int getHeight();
    Tile getTile(int x, int y);
    int getTileSize();
    SDL_Point getRenderPos();

private:
    SDL_Point getTileIndexForTileType(TileType::TileType type);
    SDL_Point getTileIndexForTileStatus(TileStatus::TileStatus status);

    const SDL_Point NULL_TEXTURE = {4, 4};

    // How many tiles wide the level is
    int _levelWidth = 0;
    // How many tiles high the level is
    int _levelHeight = 0;
    // The container containing the level tile data
    std::vector<Tile> _tilemap;
    // The rendering size of the tiles
    int _tileSize = 16;

    SDL_Point _renderPos = {0, 0};

    Spritesheet* _levelSpritesheet = nullptr;

};

#endif