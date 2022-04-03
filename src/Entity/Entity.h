#ifndef ENTITY_H
#define ENTITY_H

#include "Keyboard.h"
#include "Audio.h"
#include "Spritesheet.h"
#include "Direction.h"
#include "Weight.h"

#include <SDL.h>
#include <vector>

namespace EntityType {
    enum EntityType {
        NOVAL = -1,
        PLAYER,
        GRUNT,
        BISHOP,
    };
};

class Entity {
public:
    Entity();
    ~Entity() = default;

    virtual void tick(float timescale) = 0;
    virtual void onPlayerAction() {};
    virtual void onMove() {};
    virtual void render(int xOffset, int yOffset) = 0;

    void move(Direction::Direction dir);
    // Move overload that moves along vector path according to _moveDistance
    void move(std::vector<SDL_Point> path);
    void stop();
    void hurt(int damage);
    void completePushRequest(bool playAudio = true);

    void setPos(int x, int y);
    void setLastPos(int x, int y);
    void setRenderPos(int x, int y);
    void setDelta(int dx, int dy);
    void setMoveDistance(int moveDistance);
    void setTextureRect(SDL_Rect rect);
    void setSpritesheet(Spritesheet* spritesheet);
    void setShadowSpritesheet(Spritesheet* spritesheet);
    void setKeyboard(Keyboard* keyboard);
    void setAudioPlayer(Audio* audioPlayer);
    void setWeight(Weight::Weight weight);
    void setPathToPlayer(std::vector<SDL_Point> path);
    void setCurrentDirection(Direction::Direction dir);
    void setPlayerPos(SDL_Point pos);
    void setMoveNextMovingState(bool move);

    int getEntityId();
    EntityType::EntityType getEntityType();
    SDL_Point getPos();
    SDL_Point getLastPos();
    SDL_Point getRenderPos();
    SDL_Point getDelta();
    bool isMoving();
    int getMoveDistance();
    Spritesheet* getSpritesheet();
    Spritesheet* getShadowSpritesheet();
    Keyboard* getKeyboard();
    Audio* getAudioPlayer();
    Weight::Weight getWeight();
    bool needsPathToPlayer();
    std::vector<SDL_Point> getPathToPlayer();
    int getHealth();
    Direction::Direction getCurrentDirection();
    SDL_Point getPosFacing();
    bool requestPush();
    bool moveNextMovingState();

protected:
    void setEntityType(EntityType::EntityType type);

    const int TILE_SIZE = 16;
    bool _needsPathToPlayer = false;
    SDL_Point _playerPos = {-10, -10,};

    bool _push = false;

private:
    static int entityIdProvider;
    int _entityId;

    EntityType::EntityType _type = EntityType::NOVAL;
    Direction::Direction _currentDirection = Direction::SOUTH;

    // The entity's tile position
    SDL_Point _pos = {0, 0};
    // The entity's previous tile position
    SDL_Point _lastPos = {0, 0};
    // The entity's render position. Used for smooth movement between tiles
    SDL_Point _renderPos = {0, 0};
    int _dx = 0;
    int _dy = 0;
    int _moveDistance = 1;
    bool _moveNextMovingState = false;

    // Used for mapping the texture to the entity
    SDL_Rect _textureRect = {0, 0, 0, 0};

    int _health = 1;
    Weight::Weight _weight = Weight::MEDIUM;
    std::vector<SDL_Point> _pathToPlayer;

    // Render
    Spritesheet* _spritesheet = nullptr;
    Spritesheet* _shadowSpritesheet = nullptr;

    // Input
    Keyboard* _keyboard = nullptr;
    
    // Other
    Audio* _audioPlayer = nullptr;
};

#endif