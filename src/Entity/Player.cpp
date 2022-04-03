#include "Player.h"

Player::Player() {
    setEntityType(EntityType::PLAYER);
}

void Player::tick(float timescale) {
    handleInput();

    if(_left && !_right && !_up && !_down) move(Direction::WEST);
    if(_right && !_left && !_up && !_down) move(Direction::EAST);
    if(_up && !_down && !_left && !_right) move(Direction::NORTH); 
    if(_down && !_up && !_left && !_right) move(Direction::SOUTH);
}

void Player::onMove() {
    getAudioPlayer()->playAudio(getEntityId(), AudioSound::MOVE, 1.f);
}

void Player::render(int xOffset, int yOffset) {
    Spritesheet* ss = getSpritesheet();
    ss->setIsAnimated(false);
    ss->setIsLooped(false);
    ss->render(getPos().x * TILE_SIZE + xOffset, getPos().y * TILE_SIZE + yOffset - TILE_SIZE / 4);
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

bool Player::requestPush() {
    return _push;
}

bool Player::requestBomb() {
    return _bomb;
}

bool Player::requestRebuild() {
    return _rebuild;
}