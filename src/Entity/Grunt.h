#ifndef GRUNT_H
#define GRUNT_H

#include "Entity.h"

class Grunt : public Entity {
public:
    Grunt();
    ~Grunt() = default;

    void tick(float timescale) override;
    void onPlayerAction() override;
    void render(int xOffset, int yOffset) override;

private:

};

#endif