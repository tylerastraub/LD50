#include "GameState.h"
#include "RandomGen.h"
#include "Grunt.h"

#include <chrono>

/**
 * TODO:
 * - Start adding enemies
 * - Need to think of some sort of combat system
 * - Also probably some way to repair holes (or make map bigger)
 * - Do player assets
 * - Come up with cool level designs (need to finalize level size first)
 * 
 * IDEAS:
 * - Could add bridge building ability which gains recharges from pushing enemies off - adds
 * way to keep going but adds risk of having to get right next to enemy to be able to push.
 * - Can also add bombs which destroy tile it's on completely but kills enemies within 1 tile of it.
 * Maybe also damages all tiles around it as well?
 * - Could add snow tile that never breaks but is always on its own so you can't endlessly step on it
 * - Should add border to ice then make it iceberg over a lake
 * - Probably should give each ice tile only 2 hp instead of the 3 hp it currently has
 */

std::mt19937 RandomGen::randEng{(unsigned int) std::chrono::system_clock::now().time_since_epoch().count()};

void GameState::init() {
    // Input
    _keyboard = std::make_unique<Keyboard>();
    _controller = std::make_unique<Controller>();

    // Level
    _level = std::make_unique<Level>(9, 9);
    _level->fileTilemapWithDefaultLayout();
    _level->setLevelSpritesheet(getSpritesheet("LEVEL_TILESET"));
    _level->setRenderPos(getGameSize().x / 2 - _level->getWidth() * _level->getTileSize() / 2,
        getGameSize().y / 2 - _level->getHeight() * _level->getTileSize() / 2);

    // Player
    _player = std::make_unique<Player>();
    _player->setPos(4, 4);
    _player->setLastPos(4, 4);
    _player->setRenderPos(4 * _level->getTileSize(), 4 * _level->getTileSize());
    _player->setSpritesheet(getSpritesheet("PLAYER"));
    _player->setKeyboard(_keyboard.get());
    _player->setAudioPlayer(getAudioPlayer());

    // Entites
    _entityList.push_back(std::make_unique<Grunt>());
    _entityList.back()->setPos(7, 8);
    _entityList.back()->setLastPos(7, 8);
     _entityList.back()->setRenderPos(7 * _level->getTileSize(), 8 * _level->getTileSize());
    _entityList.back()->setAudioPlayer(getAudioPlayer());

    // Spritesheet and other init stuff for entities
    for(auto it = _entityList.begin(); it != _entityList.end(); ++it) {
        it->get()->setPlayerPos(_player->getPos());
        if(it->get()->needsPathToPlayer()) {
            it->get()->setPathToPlayer(_collisionDetector.breadthFirstSearch(it->get()->getPos(), _player->getPos(), _level.get()));
        }
        switch(it->get()->getEntityType()) {
            case EntityType::GRUNT:
                it->get()->setSpritesheet(getSpritesheet("GRUNT"));
                break;
        }
    }
}

void GameState::tick(float timescale) {
    // Reset
    if(_keyboard->isKeyPressed(SDL_SCANCODE_R)) setNextState(new GameState());

    // Move state
    if(_inMovingState) {
        _moveTimer += timescale * (float) _moveSpeed;
        if(_moveTimer >= 1) {
            _inMovingState = false;
            _moveTimer = 1.f;
        }

        if(_player->moveNextMovingState()) {
            int playerDx = _player->getPos().x - _player->getLastPos().x;
            int playerDy = _player->getPos().y - _player->getLastPos().y;
            _player->setRenderPos((_player->getLastPos().x + _moveTimer * playerDx) * _level->getTileSize(),
                (_player->getLastPos().y + _moveTimer * playerDy) * _level->getTileSize());
            _player->setMoveNextMovingState(_inMovingState);
        }

        for(auto it = _entityList.begin(); it != _entityList.end(); ++it) {
            Entity* e = it->get();
            if(e->moveNextMovingState()) {
                int eDx = e->getPos().x - e->getLastPos().x;
                int eDy = e->getPos().y - e->getLastPos().y;
                e->setRenderPos((e->getLastPos().x + _moveTimer * eDx) * _level->getTileSize(),
                    (e->getLastPos().y + _moveTimer * eDy) * _level->getTileSize());
                e->setMoveNextMovingState(_inMovingState);
            }
        }
        if(_shoveInAction && !_inMovingState && _player->getHealth() != 0) {
            _shoveInAction = false;
            if(shovingEntity == nullptr) return;
            _player->setDelta(_player->getPos().x - shovingEntity->getPos().x, _player->getPos().y - shovingEntity->getPos().y);
            _collisionDetector.checkForLevelCollisions(_player.get(), _level.get());
            shovingEntity->completePushRequest();
            _player->setMoveNextMovingState(true);
            startMovingState(DEFAULT_SHOVE_SPEED);
        }
    }
    else {
        bool playerMove = false;
        // Player tick
        if(_player->getHealth() == 0) {
            _gameOver = true;
        }
        else {
            _player->tick(timescale);
            if(_player->isMoving()) {
                SDL_Point oldPos = _player->getPos();
                _collisionDetector.checkForLevelCollisions(_player.get(), _level.get());
                playerMove = !(oldPos.x == _player->getPos().x &&
                    oldPos.y == _player->getPos().y);
                if(playerMove) {
                    _player->setMoveNextMovingState(true);
                    startMovingState(DEFAULT_MOVE_SPEED);
                }
            }
            
            if(_player->requestPush()) {
                _level->addTileEvent(_player.get(), TileEvent::PUSH);
            }
            // TODO: make sure player actually has bombs
            else if(_player->requestBomb()) {
                _level->addTileEvent(_player.get(), TileEvent::BOMB);
            }
            // TODO: make sure player can actually rebuild
            else if(_player->requestRebuild()) {
                _level->addTileEvent(_player.get(), TileEvent::REBUILD);
            }
        }
        // Entity tick
        std::vector<Entity> entityPushes;
        for(auto it = _entityList.begin(); it != _entityList.end(); ++it) {
            Entity* e = it->get();
            // Entity removal
            if(e->getHealth() == 0) {
                it = _entityList.erase(it);
                --it;
                continue;
            }
            e->tick(timescale);
            if(playerMove) {
                e->setPlayerPos(_player->getPos());
                e->onPlayerAction();
                if(e->isMoving()) {
                    SDL_Point oldPos = e->getPos();
                    _collisionDetector.checkForLevelCollisions(e, _level.get());
                    e->setMoveNextMovingState(!(oldPos.x == e->getPos().x &&
                        oldPos.y == e->getPos().y));
                }
                if(e->requestPush() && e->getPosFacing().x == _player->getPos().x && e->getPosFacing().y == _player->getPos().y) {
                    _shoveInAction = true;
                    shovingEntity = e;
                }
                // Entity moves based on old player path so that they're not insanely hard to evade
                if(e->needsPathToPlayer()) {
                    auto path = _collisionDetector.breadthFirstSearch(e->getPos(), _player->getPos(), _level.get());
                    e->setPathToPlayer(path);
                }
            }
            // Event checking
            Tile t = _level->getTile(e->getPos().x, e->getPos().y);
            if(t.getTileEvent() == TileEvent::PUSH) {
                e->setDelta(e->getPos().x - _player->getPos().x, e->getPos().y - _player->getPos().y);
                _collisionDetector.checkForLevelCollisions(e, _level.get());
                e->setMoveNextMovingState(true);
                startMovingState(DEFAULT_SHOVE_SPEED);
                if(e->needsPathToPlayer()) {
                    auto path = _collisionDetector.breadthFirstSearch(e->getPos(), _player->getPos(), _level.get());
                    e->setPathToPlayer(path);
                }
                Tile t = _level->getTile(e->getPos().x, e->getPos().y);
                t.damageTile(e->getWeight() + 1);
                _level->setTile(e->getPos().x, e->getPos().y, t);
            }
        }

        // Level update
        if(_player->requestPush()) {
            // Since by now the push should have been processed, we can remove the push event
            Tile t = _level->getTile(_player->getPosFacing().x, _player->getPosFacing().y);
            if(t.getTileEvent() == TileEvent::PUSH) t.setTileEvent(TileEvent::NOVAL);
            _level->setTile(_player->getPosFacing().x, _player->getPosFacing().y, t);
            _player->completePushRequest();
        }
    }
    // Makes level bob back and forth
    // _renderOffset.y = 3 * std::sin(_timer);

    // Input updates
    _keyboard->updateInputs();
    _controller->updateInputs();
}

void GameState::render() {
    SDL_SetRenderDrawColor(getRenderer(), 0x76, 0x9a, 0xb3, 0xFF);
    SDL_RenderClear(getRenderer());

    _level->render(_renderOffset.x, _renderOffset.y);
    if(!_gameOver) {
        _player->render(_level->getRenderPos().x + _renderOffset.x, _level->getRenderPos().y + _renderOffset.y);
    }

    SDL_SetRenderDrawColor(getRenderer(), 0xFF, 0x00, 0x00, 0x64);
    for(auto e = _entityList.begin(); e != _entityList.end(); ++e) {
        e->get()->render(_level->getRenderPos().x + _renderOffset.x,
            _level->getRenderPos().y + _renderOffset.y);
        SDL_Rect r = {e->get()->getPosFacing().x * 16 + _level->getRenderPos().x + _renderOffset.x,
            e->get()->getPosFacing().y * 16 + _level->getRenderPos().y + _renderOffset.y,
            16,
            16};
        SDL_RenderFillRect(getRenderer(), &r);
    }

    SDL_RenderPresent(getRenderer());
}

void GameState::handleControllerButtonInput(SDL_Event e) {
    _controller->updateButtonInputs(e);
}

void GameState::handleControllerAxisInput(SDL_Event e) {
    _controller->updateAxisInputs(e);
}

void GameState::startMovingState(int moveSpeed) {
    _inMovingState = true;
    _moveTimer = 0;
    _moveSpeed = moveSpeed;
}