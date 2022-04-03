#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "State.h"
#include "Keyboard.h"
#include "Controller.h"
#include "Level.h"
#include "Player.h"
#include "CollisionDetector.h"
#include "Bomb.h"

#include <list>

class GameState: public State {
public:
    GameState() = default;
    ~GameState() = default;

    void init() override;
    void tick(float timescale) override;
    void render() override;
    void handleKeyboardInput(SDL_Event e) override {};
    void handleControllerButtonInput(SDL_Event e) override;
    void handleControllerAxisInput(SDL_Event e) override;

private:
    void startMovingState(int moveSpeed);
    void tickBombTimers();
    void addEnemySpawn();

    const int DEFAULT_MOVE_SPEED = 4;
    const int DEFAULT_SHOVE_SPEED = 12;
    const int DEFAULT_SPAWN_SPEED = 1;

    std::unique_ptr<Keyboard> _keyboard = nullptr;
    std::unique_ptr<Controller> _controller = nullptr;

    SDL_Point _renderOffset = {0, 0};

    std::unique_ptr<Level> _level = nullptr;
    std::unique_ptr<Player> _player = nullptr;

    std::list<std::unique_ptr<Entity>> _entityList;
    CollisionDetector _collisionDetector;

    float _moveTimer = 0;
    // 1 / _moveSpeed = time in seconds (for instance _moveSpeed = 4 means 0.25 sec move time)
    int _moveSpeed = 4;

    bool _gameOver = false;

    bool _inMovingState = false;
    bool _spawnEnemy = false;

    bool _shoveInAction = false;
    Entity* shovingEntity = nullptr;
    bool _shoveKill = false;

    std::list<Bomb> _bombList;
};

#endif