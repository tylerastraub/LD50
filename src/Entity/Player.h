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

    void setBombCount(int numOfBombs);
    void setRebuildCount(int numOfRebuilds);

    bool requestBomb();
    bool requestRebuild();
    int getNumOfBombs();
    int getNumOfRebuilds();

private:
    bool _left = false;
    bool _right = false;
    bool _up = false;
    bool _down = false;
    
    bool _bomb = false;
    bool _rebuild = false;

    int _numOfBombs = 3;
    int _numOfRebuilds = 3;
};

#endif