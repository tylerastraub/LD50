#ifndef TANK_H
#define TANK_H

#include "Entity.h"

class Tank : public Entity {
public:
    Tank();
    ~Tank() = default;

    void tick(float timescale) override;
    void onPlayerAction() override;
    void render(int xOffset, int yOffset) override;

private:
    const int MS_BETWEEN_WALK_FRAMES = 89;
    const int MS_BETWEEN_IDLE_FRAMES = 249;
    const int NUM_OF_WALK_FRAMES = 4;
    const int NUM_OF_IDLE_FRAMES = 2;
};

#endif