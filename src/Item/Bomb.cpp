#include "Bomb.h"

Bomb::Bomb(int x, int y, int timer) : _pos({x, y}), _timer(timer), _startingTimer(timer) {}

void Bomb::render(int xOffset, int yOffset) {
    _spritesheet->setTileIndex(SDL_GetTicks() / _msBetweenFrames % 2, 0);
    _spritesheet->render(_pos.x * TILE_SIZE + xOffset, _pos.y * TILE_SIZE + yOffset);
}

void Bomb::setSpritesheet(Spritesheet* spritesheet) {
    _spritesheet = spritesheet;
}

void Bomb::tickTimer() {
    --_timer;
    if(_timer > 0) _msBetweenFrames = (float) _timer / (float) _startingTimer * (float) _startingMsBetweenFrames;
}

SDL_Point Bomb::getPos() {
    return _pos;
}

bool Bomb::bombExploded() {
    return _timer < 1;
}

std::vector<SDL_Point> Bomb::getSurroundingPoints() {
    std::vector<SDL_Point> result = {
        {_pos.x + 1, _pos.y},
        {_pos.x + 1, _pos.y + 1},
        {_pos.x + 1, _pos.y - 1},
        {_pos.x - 1, _pos.y},
        {_pos.x - 1, _pos.y + 1},
        {_pos.x - 1, _pos.y - 1},
        {_pos.x, _pos.y + 1},
        {_pos.x, _pos.y - 1}
    };
    return result;
}