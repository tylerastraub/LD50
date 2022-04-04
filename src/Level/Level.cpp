#include "Level.h"

#include <iostream>

Level::Level(int w, int h) {
    _levelWidth = w;
    _levelHeight = h;
    for(int i = 0; i < _levelWidth * _levelHeight; ++i) {
        _tilemap.push_back(Tile(TileType::NOVAL));
    }
}

void Level::render(int xOffset, int yOffset) {
    int tileSize = _levelSpritesheet->getTileWidth();
    for(size_t i = 0; i < _tilemap.size(); ++i) {
        Tile tile = _tilemap[i];
        int xPos = i % _levelHeight;
        int yPos = i / _levelHeight;
        // Render tile
        if(tile.getTileStatus() != TileStatus::BROKEN) {
            SDL_Point ssCoords = getTileIndexForTileType(tile.getTileType());
            _levelSpritesheet->setTileIndex(ssCoords.x, ssCoords.y);
            _levelSpritesheet->render(_renderPos.x + xOffset + xPos * tileSize,
                _renderPos.y + yOffset + yPos * tileSize);
        }
        // Render tile cracks
        if(tile.getTileStatus() != TileStatus::HEALTHY) {
            SDL_Point overlayCoords = getTileIndexForTileStatus(tile.getTileStatus());
            _levelSpritesheet->setTileIndex(overlayCoords.x, overlayCoords.y);
            _levelSpritesheet->render(_renderPos.x + xOffset + xPos * tileSize,
                _renderPos.y + yOffset + yPos * tileSize);
        }
        if(tile.getTileEvent() == TileEvent::PUSH) {
            _levelSpritesheet->setTileIndex(3, 3);
            _levelSpritesheet->render(_renderPos.x + xOffset + xPos * tileSize,
                _renderPos.y + yOffset + yPos * tileSize);
        }
    }
    for(int x = -1; x <= _levelWidth; ++x) {
        if(x == -1) {
            _levelSpritesheet->setTileIndex(0, 1);
            _levelSpritesheet->render(_renderPos.x + xOffset + x * tileSize,
                _renderPos.y + yOffset - tileSize);
            _levelSpritesheet->setTileIndex(0, 3);
            _levelSpritesheet->render(_renderPos.x + xOffset + x * tileSize,
                _renderPos.y + yOffset + _levelHeight * tileSize);
        }
        else if(x == _levelWidth) {
            _levelSpritesheet->setTileIndex(2, 1);
            _levelSpritesheet->render(_renderPos.x + xOffset + x * tileSize,
                _renderPos.y + yOffset - tileSize);
            _levelSpritesheet->setTileIndex(2, 3);
            _levelSpritesheet->render(_renderPos.x + xOffset + x * tileSize,
                _renderPos.y + yOffset + _levelHeight * tileSize);
        }
        else {
            _levelSpritesheet->setTileIndex(1, 1);
            _levelSpritesheet->render(_renderPos.x + xOffset + x * tileSize,
                _renderPos.y + yOffset - tileSize);
            _levelSpritesheet->setTileIndex(1, 3);
            _levelSpritesheet->render(_renderPos.x + xOffset + x * tileSize,
                _renderPos.y + yOffset + _levelHeight * tileSize);
        }
    }
    for(int y = 0; y < _levelHeight; ++y) {
        _levelSpritesheet->setTileIndex(0, 2);
        _levelSpritesheet->render(_renderPos.x + xOffset - tileSize,
            _renderPos.y + yOffset + y * tileSize);
        _levelSpritesheet->setTileIndex(2, 2);
        _levelSpritesheet->render(_renderPos.x + xOffset + _levelWidth * tileSize,
            _renderPos.y + yOffset + y * tileSize);
    }
}

// Fills level with all healthy ground tiles and a few snow tiles here and there
void Level::fileTilemapWithDefaultLayout() {
    for(size_t i = 0; i < _tilemap.size(); ++i) {
        int xPos = i % _levelHeight;
        int yPos = i / _levelHeight;
        _validSpawnTiles.push_back({xPos, yPos});
        _tilemap[i].setTileType(TileType::ICE);
        _tilemap[i].setTileStatus(TileStatus::HEALTHY);
    }
    Tile snowTile(TileType::SNOW, TileStatus::HEALTHY);
    setTile(_levelWidth / 2, _levelHeight / 2, snowTile);
    setTile(1, 1, snowTile);
    setTile(_levelWidth - 2, 1, snowTile);
    setTile(1, _levelHeight - 2, snowTile);
    setTile(_levelWidth - 2, _levelHeight - 2, snowTile);
}

void Level::onTileMovedFrom(int x, int y, Entity* e, bool wasPushed) {
    Tile t = getTile(x, y);
    if(t.canBeDamaged() && !wasPushed) {
        t.damageTile(e->getWeight());
        setTile(x, y, t);
    }
}

void Level::onTileMovedTo(int x, int y, Entity* e, bool wasPushed) {
    Tile t = getTile(x, y);
    if(t.canBeDamaged() && wasPushed) {
        int extraDamage = 1;
        if(e->getEntityType() == EntityType::PLAYER) extraDamage = 0;
        t.damageTile(e->getWeight() + extraDamage);
        setTile(x, y, t);
    }
}

void Level::setTile(int x, int y, Tile tile) {
    if(x < 0 || x >= _levelWidth || y < 0 || y >= _levelHeight) {
        std::cout << "Error: attempting to set tile out of bounds at " << x << ", " << y << std::endl;
    }
    else {
        if(tile.getTileStatus() == TileStatus::BROKEN) {
            for(auto it = _validSpawnTiles.begin(); it != _validSpawnTiles.end(); ++it) {
                if(it->x == x && it->y == y) {
                    _validSpawnTiles.erase(it);
                    break;
                }
            }
        }
        else if(tile.getTileType() != TileType::NOVAL && tile.getTileType() != TileType::WALL) {
            bool tileAlreadyValid = false;
            for(auto it = _validSpawnTiles.begin(); it != _validSpawnTiles.end(); ++it) {
                if(it->x == x && it->y == y) {
                    tileAlreadyValid = true;
                    break;
                }
            }
            if(!tileAlreadyValid) {
                _validSpawnTiles.push_back({x, y});
            }
        }
        int index = y * _levelWidth + x;
        _tilemap[index] = tile;
    }
}

void Level::addTileEvent(Entity* e, TileEvent::TileEvent event) {
    SDL_Point p = e->getPosFacing();
    switch(event) {
        case TileEvent::PUSH: {
            Tile t = getTile(p.x, p.y);
            if(t.getTileType() != TileType::NOVAL && t.getTileStatus() != TileStatus::BROKEN) {
                t.setTileEvent(TileEvent::PUSH);
            }
            setTile(p.x, p.y, t);
            break;
        }
        case TileEvent::BOMB: {
            Tile t = getTile(e->getPos().x, e->getPos().y);
            t.setTileEvent(TileEvent::BOMB); // Don't need to check since player is on this tile
            setTile(e->getPos().x, e->getPos().y, t);
            break;
        }
        case TileEvent::REBUILD: {
            Tile t = getTile(p.x, p.y);
            if(t.getTileStatus() == TileStatus::BROKEN) {
                t.setTileStatus(TileStatus::HEALTHY);
            }
            setTile(p.x, p.y, t);
            break;
        }
    }
}

void Level::setLevelSpritesheet(Spritesheet* spritesheet) {
    _levelSpritesheet = spritesheet;
}

void Level::setRenderPos(int x, int y) {
    _renderPos = {x, y};
}

int Level::getWidth() {
    return _levelWidth;
}

int Level::getHeight() {
    return _levelHeight;
}

Tile Level::getTile(int x, int y) {
    if(x < 0 || x >= _levelWidth || y < 0 || y >= _levelHeight) {
        return Tile(TileType::NOVAL, TileStatus::NOVAL);
    }
    int index = y * _levelWidth + x;
    return _tilemap[index];
}

int Level::getTileSize() {
    return _tileSize;
}

SDL_Point Level::getRenderPos() {
    return _renderPos;
}

std::vector<SDL_Point> Level::getValidSpawnTiles() {
    return _validSpawnTiles;
}

/**
 * ======== PRIVATE METHODS ========
 */

SDL_Point Level::getTileIndexForTileType(TileType::TileType type) {
    switch(type) {
        case TileType::ICE:
            return {1, 2};
        case TileType::SNOW:
            return {3, 1};
        default:
            return NULL_TEXTURE;
    }
}

SDL_Point Level::getTileIndexForTileStatus(TileStatus::TileStatus status) {
    switch(status) {
        case TileStatus::DAMAGE_LIGHT:
            return {0, 0};
        case TileStatus::DAMAGE_MEDIUM:
            return {1, 0};
        // case TileStatus::DAMAGE_HEAVY:
        //     return {2, 0};
        case TileStatus::BROKEN:
            return {3, 0};
        default:
            return NULL_TEXTURE;
    }
}