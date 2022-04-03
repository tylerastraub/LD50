#include "Grunt.h"

Grunt::Grunt() {
    setWeight(Weight::LIGHT);
    setEntityType(EntityType::GRUNT);
    _needsPathToPlayer = true;
}

void Grunt::tick(float timescale) {

}

void Grunt::onPlayerAction() {
    move(getPathToPlayer());
    _push = true;
}

void Grunt::render(int xOffset, int yOffset) {
    Spritesheet* ss = getSpritesheet();
    ss->setIsAnimated(false);
    ss->setIsLooped(false);
    ss->render(getRenderPos().x + xOffset, getRenderPos().y + yOffset);
}