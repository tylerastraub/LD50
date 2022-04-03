#include "Entity.h"

Entity::Entity() {
    entityIdProvider++;
    _entityId = entityIdProvider;
}

void Entity::move(Direction::Direction dir) {
    _currentDirection = dir;
    switch(dir) {
        case Direction::NORTH:
            setDelta(0, -1);
            break;
        case Direction::SOUTH:
            setDelta(0, 1);
            break;
        case Direction::EAST:
            setDelta(1, 0);
            break;
        case Direction::WEST:
            setDelta(-1, 0);
            break;
        case Direction::NORTHEAST:
            setDelta(1, -1);
            break;
        case Direction::NORTHWEST:
            setDelta(-1, -1);
            break;
        case Direction::SOUTHEAST:
            setDelta(1, 1);
            break;
        case Direction::SOUTHWEST:
            setDelta(-1, 1);
            break;
        default:
            break;
    }
}

void Entity::move(std::vector<SDL_Point> path) {
    if(path.empty()) return;
    SDL_Point p = *path.begin();
    int dx = p.x - getPos().x;
    int dy = p.y - getPos().y;
    setDelta(dx * _moveDistance, dy * _moveDistance);

    if(dx < 0) {
        if(dy < 0) {
            _currentDirection = Direction::NORTHWEST;
        }
        else if(dy > 0) {
            _currentDirection = Direction::SOUTHWEST;
        }
        else {
            _currentDirection = Direction::WEST;
        }
    }
    else if(dx > 0) {
        if(dy < 0) {
            _currentDirection = Direction::NORTHEAST;
        }
        else if(dy > 0) {
            _currentDirection = Direction::SOUTHEAST;
        }
        else {
            _currentDirection = Direction::EAST;
        }
    }
    else if(dy != 0) {
        std::cout << "Entity Type " << _type << " with dy " << dy << std::endl;
        if(dy < 0) {
            _currentDirection = Direction::NORTH;
        }
        else if(dy > 0) {
            _currentDirection = Direction::SOUTH;
        }
    }
}

void Entity::stop() {
    _dx = 0;
    _dy = 0;
}

void Entity::hurt(int damage) {
    _health -= damage;
    if(_health < 0) _health = 0;
}

void Entity::completePushRequest(bool playAudio) {
    _push = false;
    if(playAudio) _audioPlayer->playAudio(_entityId, AudioSound::SHOVE, 1.f);
}

void Entity::setEntityType(EntityType::EntityType type) {
    _type = type;
}

void Entity::setPos(int x, int y) {
    _lastPos = _pos;
    _pos = {x, y};
}

void Entity::setLastPos(int x, int y) {
    _lastPos = {x, y};
}

void Entity::setRenderPos(int x, int y) {
    _renderPos = {x, y - TILE_SIZE / 4};
}

void Entity::setDelta(int dx, int dy) {
    _dx = dx;
    _dy = dy;
}

void Entity::setMoveDistance(int moveDistance) {
    _moveDistance = moveDistance;
}

void Entity::setTextureRect(SDL_Rect rect) {
    _textureRect = rect;
}

void Entity::setSpritesheet(Spritesheet* spritesheet) {
    _spritesheet = spritesheet;
}


void Entity::setShadowSpritesheet(Spritesheet* spritesheet) {
    _shadowSpritesheet = spritesheet;
}

void Entity::setKeyboard(Keyboard* keyboard) {
    _keyboard = keyboard;
}

void Entity::setAudioPlayer(Audio* audio) {
    _audioPlayer = audio;
}

void Entity::setWeight(Weight::Weight weight) {
    _weight = weight;
}

void Entity::setPathToPlayer(std::vector<SDL_Point> path) {
    _pathToPlayer = path;
}

void Entity::setCurrentDirection(Direction::Direction dir) {
    _currentDirection = dir;
}

void Entity::setPlayerPos(SDL_Point pos) {
    _playerPos = pos;
}

void Entity::setMoveNextMovingState(bool move) {
    _moveNextMovingState = move;
}

int Entity::getEntityId() {
    return _entityId;
}

EntityType::EntityType Entity::getEntityType() {
    return _type;
}

SDL_Point Entity::getPos() {
    return _pos;
}

SDL_Point Entity::getLastPos() {
    return _lastPos;
}

SDL_Point Entity::getRenderPos() {
    return _renderPos;
}

SDL_Point Entity::getDelta() {
    return {_dx, _dy};
}

bool Entity::isMoving() {
    return (_dx != 0 || _dy != 0);
}

int Entity::getMoveDistance() {
    return _moveDistance;
}

Spritesheet* Entity::getSpritesheet() {
    return _spritesheet;
}

Spritesheet* Entity::getShadowSpritesheet() {
    return _shadowSpritesheet;
}

Keyboard* Entity::getKeyboard() {
    return _keyboard;
}

Audio* Entity::getAudioPlayer() {
    return _audioPlayer;
}

Weight::Weight Entity::getWeight() {
    return _weight;
}

bool Entity::needsPathToPlayer() {
    return _needsPathToPlayer;
}

std::vector<SDL_Point> Entity::getPathToPlayer() {
    return _pathToPlayer;
}

int Entity::getHealth() {
    return _health;
}

Direction::Direction Entity::getCurrentDirection() {
    return _currentDirection;
}

SDL_Point Entity::getPosFacing() {
    switch(_currentDirection) {
        case Direction::NORTH:
            return {_pos.x, _pos.y - 1};
        case Direction::SOUTH:
            return {_pos.x, _pos.y + 1};
        case Direction::EAST:
            return {_pos.x + 1, _pos.y};
        case Direction::WEST:
            return {_pos.x - 1, _pos.y};
        case Direction::NORTHEAST:
            return {_pos.x + 1, _pos.y - 1};
        case Direction::NORTHWEST:
            return {_pos.x - 1, _pos.y - 1};
        case Direction::SOUTHEAST:
            return {_pos.x + 1, _pos.y + 1};
        case Direction::SOUTHWEST:
            return {_pos.x - 1, _pos.y + 1};
        default:
            return _pos;
    }
}

bool Entity::requestPush() {
    return _push;
}

bool Entity::moveNextMovingState() {
    return _moveNextMovingState;
}