#include "Player.h"

Player::Player() {
    setEntityType(EntityType::PLAYER);
    setSpawning(false);
}

void Player::tick(float timescale) {
    handleInput();

    if(_left && !_right && !_up && !_down) {
        move(Direction::WEST);
        setCurrentDirection(Direction::WEST);
    }
    if(_right && !_left && !_up && !_down) {
        move(Direction::EAST);
        setCurrentDirection(Direction::EAST);
    }
    if(_up && !_down && !_left && !_right) {
        move(Direction::NORTH); 
        setCurrentDirection(Direction::NORTH);
    }
    if(_down && !_up && !_left && !_right) {
        move(Direction::SOUTH);
        setCurrentDirection(Direction::SOUTH);
    }
}

void Player::onMove() {
    getAudioPlayer()->playAudio(getEntityId(), AudioSound::MOVE, 1.f);
}

void Player::render(int xOffset, int yOffset) {
    Spritesheet* ss = getShadowSpritesheet();
    ss->setTileIndex(0, 0);
    ss->render(getRenderPos().x + xOffset, getRenderPos().y + yOffset + 1);

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

void Player::handleInput() {
    Keyboard* k = getKeyboard();
    _left = (k->isKeyPressed(SDL_SCANCODE_LEFT) || k->isKeyPressed(SDL_SCANCODE_A));
    _right = (k->isKeyPressed(SDL_SCANCODE_RIGHT) || k->isKeyPressed(SDL_SCANCODE_D));
    _up = (k->isKeyPressed(SDL_SCANCODE_UP) || k->isKeyPressed(SDL_SCANCODE_W));
    _down = (k->isKeyPressed(SDL_SCANCODE_DOWN) || k->isKeyPressed(SDL_SCANCODE_S));
    _push = (k->isKeyPressed(SDL_SCANCODE_Z) || k->isKeyPressed(SDL_SCANCODE_I));
    _bomb = (k->isKeyPressed(SDL_SCANCODE_X) || k->isKeyPressed(SDL_SCANCODE_O));
    _rebuild = (k->isKeyPressed(SDL_SCANCODE_C) || k->isKeyPressed(SDL_SCANCODE_P));
}

void Player::setBombCount(int numOfBombs) {
    _numOfBombs = numOfBombs;
}

void Player::setRebuildCount(int numOfRebuilds) {
    _numOfRebuilds = numOfRebuilds;
}

bool Player::requestBomb() {
    return _bomb;
}

bool Player::requestRebuild() {
    return _rebuild;
}

int Player::getNumOfBombs() {
    return _numOfBombs;
}

int Player::getNumOfRebuilds() {
    return _numOfRebuilds;
}