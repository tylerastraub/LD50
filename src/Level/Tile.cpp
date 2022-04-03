#include "Tile.h"

Tile::Tile(
    TileType::TileType tileType,
    TileStatus::TileStatus tileStatus,
    TileEvent::TileEvent tileEvent
    ) : _tileType(tileType), _tileStatus(tileStatus), _tileEvent(tileEvent) {}

void Tile::damageTile(int damage) {
    if(_tileStatus != TileStatus::NOVAL) {
        int status = (int) _tileStatus + damage;
        if(status > (int) TileStatus::BROKEN) status = (int) TileStatus::BROKEN;
        setTileStatus((TileStatus::TileStatus) status);
    }
}

void Tile::setTileType(TileType::TileType type) {
    _tileType = type;
}

void Tile::setTileStatus(TileStatus::TileStatus status) {
    _tileStatus = status;
}

void Tile::setTileEvent(TileEvent::TileEvent event) {
    _tileEvent = event;
}

TileType::TileType Tile::getTileType() {
    return _tileType;
}

TileStatus::TileStatus Tile::getTileStatus() {
    return _tileStatus;
}

TileEvent::TileEvent Tile::getTileEvent() {
    return _tileEvent;
}

bool Tile::canBeDamaged() {
    switch(_tileType) {
        case TileType::ICE:
            return true;
    }

    return false;
}