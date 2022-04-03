#include "Tank.h"

Tank::Tank() {
    setWeight(Weight::MEDIUM); // experiment with this at heavy as well
    setEntityType(EntityType::TANK);
    _needsPathToPlayer = true;
}

void Tank::tick(float timescale) {

}

void Tank::onPlayerAction() {
    move(getPathToPlayer());
    _push = true;
}

void Tank::render(int xOffset, int yOffset) {
    Spritesheet* ss = getShadowSpritesheet();
    ss->setTileIndex(0, 0);
    ss->render(getRenderPos().x + xOffset, getRenderPos().y + yOffset + 2);

    ss = getSpritesheet();
    int yIndex = 0;
    int xIndex = 0;
    bool spriteFlip = (getCurrentDirection() == Direction::WEST);
    if(moveNextMovingState()) {
        xIndex = SDL_GetTicks() / MS_BETWEEN_WALK_FRAMES % NUM_OF_WALK_FRAMES;
        if(getCurrentDirection() == Direction::SOUTH) {
            yIndex = 3;
        }
        else if(getCurrentDirection() == Direction::EAST || getCurrentDirection() == Direction::WEST) {
            yIndex = 4;
        }
        else if(getCurrentDirection() == Direction::NORTH) {
            yIndex = 5;
        }
    }
    else {
        xIndex = SDL_GetTicks() / MS_BETWEEN_IDLE_FRAMES % NUM_OF_IDLE_FRAMES;
        if(getCurrentDirection() == Direction::SOUTH) {
            yIndex = 0;
        }
        else if(getCurrentDirection() == Direction::EAST || getCurrentDirection() == Direction::WEST) {
            yIndex = 1;
        }
        else if(getCurrentDirection() == Direction::NORTH) {
            yIndex = 2;
        }
    }
    ss->setTileIndex(xIndex, yIndex);
    // While SDL_RendererFlip is technically an enum, we can get the values we want with the spriteFlip bool
    ss->render(getRenderPos().x + xOffset, getRenderPos().y + yOffset, (SDL_RendererFlip) spriteFlip);
}