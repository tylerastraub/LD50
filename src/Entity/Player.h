#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"

class Player : public Entity {
public:
    Player();
    ~Player() = default;

    void tick(float timescale) override;
    void onMove() override;
    void render(int xOffset, int yOffset) override;

    void handleInput();

    bool requestPush();
    bool requestBomb();
    bool requestRebuild();

private:
    bool _left = false;
    bool _right = false;
    bool _up = false;
    bool _down = false;
    
    bool _push = false;
    bool _bomb = false;
    bool _rebuild = false;
};

#endif