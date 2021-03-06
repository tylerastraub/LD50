#ifndef TILE_H
#define TILE_H

#include "TileType.h"
#include "TileStatus.h"
#include "TileEvent.h"

class Tile {
public:
    Tile(
        TileType::TileType tileType,
        TileStatus::TileStatus tileStatus = TileStatus::HEALTHY,
        TileEvent::TileEvent tileEvent = TileEvent::NOVAL);
    ~Tile() = default;

    void damageTile(int damage);

    void setTileType(TileType::TileType type);
    void setTileStatus(TileStatus::TileStatus status);
    void setTileEvent(TileEvent::TileEvent event);
    void setEntityOnTile(bool onTile);

    TileType::TileType getTileType();
    TileStatus::TileStatus getTileStatus();
    TileEvent::TileEvent getTileEvent();
    bool canBeDamaged();
    bool isEntityOnTile();

private:
    TileType::TileType _tileType = TileType::NOVAL;
    TileStatus::TileStatus _tileStatus = TileStatus::NOVAL;
    TileEvent::TileEvent _tileEvent = TileEvent::NOVAL;
    // Whether or not the tile has an entity on it
    bool _entityOnTile = false;
};

#endif