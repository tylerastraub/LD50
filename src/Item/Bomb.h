#ifndef BOMB_H
#define BOMB_H

#include "Spritesheet.h"

#include <SDL.h>
#include <vector>

class Bomb {
public:
    Bomb(int x, int y, int timer);
    ~Bomb() = default;

    void tickTimer();
    void render(int xOffset, int yOffset);

    void setSpritesheet(Spritesheet* spritesheet);
    
    SDL_Point getPos();
    bool bombExploded();
    std::vector<SDL_Point> getSurroundingPoints();

private:
    const int TILE_SIZE = 16;

    Spritesheet* _spritesheet = nullptr;

    SDL_Point _pos = {0, 0};
    int _startingTimer = 0;
    int _timer = 0;
    int _startingMsBetweenFrames = 249;
    int _msBetweenFrames = 249;

};

#endif