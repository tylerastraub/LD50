#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "State.h"
#include "Keyboard.h"
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

private:
    void startMovingState(float moveSpeed);
    void tickBombTimers();
    void addEnemySpawn();

    const float DEFAULT_MOVE_SPEED = 4.f;
    const float DEFAULT_SHOVE_SPEED = 12.f;
    const float DEFAULT_SPAWN_SPEED = 0.8f;

    std::unique_ptr<Keyboard> _keyboard = nullptr;

    SDL_Point _renderOffset = {0, 0};

    std::unique_ptr<Level> _level = nullptr;
    std::unique_ptr<Player> _player = nullptr;

    std::list<std::unique_ptr<Entity>> _entityList;
    CollisionDetector _collisionDetector;

    float _moveTimer = 0;
    // 1 / _moveSpeed = time in seconds (for instance _moveSpeed = 4 means 0.25 sec move time)
    float _moveSpeed = 4;

    bool _gameOver = false;

    bool _inMovingState = false;
    bool _spawnEnemy = false;

    bool _shoveInAction = false;
    Entity* shovingEntity = nullptr;
    int _shoveKills = 0;

    std::list<Bomb> _bombList;

    // Scorekeeping
    int _score = 0;
    int _moves = 0;
    const int SHOVE_SCORE = 100;
    const int SHOVE_KILL_SCORE = 2000;
    const int BOMB_KILL_SCORE = 3000;
    const int REBUILD_SCORE = 1500;

    int _movesSinceLastEnemySpawn = 0;
    int _currentSpawnWaitTime = 0;
    const int MIN_MOVES_BETWEEN_SPAWNS = 5;
    const int MAX_MOVES_BETWEEN_SPAWNS = 10;
    const int GRUNT_SPAWN_CHANCE = 4;
    const int BISHOP_SPAWN_CHANCE = 3;
    const int TANK_SPAWN_CHANCE = 1;
};

#endif