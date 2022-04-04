#include "Bishop.h"

Bishop::Bishop() {
    setEntityType(EntityType::BISHOP);
    setWeight(Weight::LIGHT);
    setCurrentDirection(Direction::SOUTHEAST);
    _needsPathToPlayer = true;
}

void Bishop::tick(float timescale) {

}

void Bishop::onPlayerAction() {
    move(getPathToPlayer());
    _push = true;
}

void Bishop::render(int xOffset, int yOffset) {
    Spritesheet* ss = getShadowSpritesheet();
    ss->setTileIndex(0, 0);
    if(isSpawning()) {
        ss->render(getRenderPos().x + xOffset, getPos().y * TILE_SIZE - TILE_SIZE / 4 + yOffset);
    }
    else {
        ss->render(getRenderPos().x + xOffset, getRenderPos().y + yOffset);
    }

    ss = getSpritesheet();
    int yIndex = 0;
    int xIndex = 0;
    bool spriteFlip = (getCurrentDirection() == Direction::NORTHWEST ||
        getCurrentDirection() == Direction::SOUTHWEST);
    if(moveNextMovingState() && !isSpawning()) {
        xIndex = SDL_GetTicks() / MS_BETWEEN_WALK_FRAMES % NUM_OF_WALK_FRAMES;
        if(getCurrentDirection() == Direction::SOUTHEAST ||
            getCurrentDirection() == Direction::SOUTHWEST) {
            yIndex = 2;
        }
        else if(getCurrentDirection() == Direction::NORTHEAST ||
            getCurrentDirection() == Direction::NORTHWEST) {
            yIndex = 3;
        }
    }
    else {
        xIndex = SDL_GetTicks() / MS_BETWEEN_IDLE_FRAMES % NUM_OF_IDLE_FRAMES;
        if(getCurrentDirection() == Direction::SOUTHEAST ||
            getCurrentDirection() == Direction::SOUTHWEST) {
            yIndex = 0;
        }
        else if(getCurrentDirection() == Direction::NORTHEAST ||
            getCurrentDirection() == Direction::NORTHWEST) {
            yIndex = 1;
        }
    }
    ss->setTileIndex(xIndex, yIndex);
    // While SDL_RendererFlip is technically an enum, we can get the values we want with the spriteFlip bool
    ss->render(getRenderPos().x + xOffset, getRenderPos().y + yOffset, (SDL_RendererFlip) spriteFlip);
}