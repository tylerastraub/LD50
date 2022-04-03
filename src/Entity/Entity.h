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

    void setPos(int x, int y);
    void setDelta(int dx, int dy);
    void setMoveDistance(int moveDistance);
    void setTextureRect(SDL_Rect rect);
    void setSpritesheet(Spritesheet* spritesheet);
    void setKeyboard(Keyboard* keyboard);
    void setAudioPlayer(Audio* audioPlayer);
    void setWeight(Weight::Weight weight);
    void setPathToPlayer(std::vector<SDL_Point> path);
    void setCurrentDirection(Direction::Direction dir);

    int getEntityId();
    EntityType::EntityType getEntityType();
    SDL_Point getPos();
    SDL_Point getDelta();
    bool isMoving();
    int getMoveDistance();
    Spritesheet* getSpritesheet();
    Keyboard* getKeyboard();
    Audio* getAudioPlayer();
    Weight::Weight getWeight();
    bool needsPathToPlayer();
    std::vector<SDL_Point> getPathToPlayer();
    int getHealth();
    Direction::Direction getCurrentDirection();
    SDL_Point getPosFacing();

protected:
    void setEntityType(EntityType::EntityType type);

    const int TILE_SIZE = 16;
    bool _needsPathToPlayer = false;

private:
    static int entityIdProvider;
    int _entityId;

    EntityType::EntityType _type = EntityType::NOVAL;
    Direction::Direction _currentDirection = Direction::SOUTH;

    // The entity's tile position
    SDL_Point _pos = {0, 0};
    int _dx = 0;
    int _dy = 0;
    int _moveDistance = 1;

    // Used for mapping the texture to the entity
    SDL_Rect _textureRect = {0, 0, 0, 0};

    int _health = 1;
    Weight::Weight _weight = Weight::MEDIUM;
    std::vector<SDL_Point> _pathToPlayer;

    // Render
    Spritesheet* _spritesheet = nullptr;

    // Input
    Keyboard* _keyboard = nullptr;
    
    // Other
    Audio* _audioPlayer = nullptr;
};

#endif