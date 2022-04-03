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
}

void Grunt::render(int xOffset, int yOffset) {
    Spritesheet* ss = getSpritesheet();
    ss->setIsAnimated(false);
    ss->setIsLooped(false);
    ss->render(getPos().x * TILE_SIZE + xOffset, getPos().y * TILE_SIZE + yOffset - TILE_SIZE / 4);
}