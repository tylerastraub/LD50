#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "State.h"
#include "Keyboard.h"
#include "Controller.h"
#include "Level.h"
#include "Player.h"
#include "CollisionDetector.h"

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
    std::unique_ptr<Keyboard> _keyboard = nullptr;
    std::unique_ptr<Controller> _controller = nullptr;

    SDL_Point _renderOffset = {0, 0};

    std::unique_ptr<Level> _level = nullptr;
    std::unique_ptr<Player> _player = nullptr;

    std::list<std::unique_ptr<Entity>> _entityList;
    CollisionDetector _collisionDetector;

    bool _gameOver = false;
};

#endif