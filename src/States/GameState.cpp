#include "GameState.h"
#include "RandomGen.h"
#include "Grunt.h"
#include "Bishop.h"

#include <chrono>

/**
 * TODO:
 * - Add enemy: Tank. Moves every other turn but does 2 damage to each tile
 * - Add enemy spawning
 * - Add death sound (falling)
 * - Refine HUD more
 * - Gamify with score/moves taken
 * EXTRAS: (if have time)
 * - Fix bug where bomb can be placed on damaged tile then stays after tile gets broken
 * - Enemy idea: Wildcard. Move in a random direction every turn
 * - Bishop not great at knowing when to shove player, could work on that
 * - Add bomb explosion sprite
 * - Add falling effect (shrink sprite?)
 * - Add water lines (if have time add them around iceberg too)
 */

std::mt19937 RandomGen::randEng{(unsigned int) std::chrono::system_clock::now().time_since_epoch().count()};

void GameState::init() {
    // Input
    _keyboard = std::make_unique<Keyboard>();

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
    _player->setShadowSpritesheet(getSpritesheet("SHADOW"));
    _player->setKeyboard(_keyboard.get());
    _player->setAudioPlayer(getAudioPlayer());

    // Entites
    _entityList.push_back(std::make_unique<Grunt>());
    _entityList.back()->setPos(7, 8);
    _entityList.back()->setLastPos(7, 8);
    _entityList.back()->setRenderPos(7 * _level->getTileSize(), 8 * _level->getTileSize());
    _entityList.back()->setAudioPlayer(getAudioPlayer());
    _entityList.push_back(std::make_unique<Bishop>());
    _entityList.back()->setPos(2, 2);
    _entityList.back()->setLastPos(2, 2);
    _entityList.back()->setRenderPos(2 * _level->getTileSize(), 2 * _level->getTileSize());
    _entityList.back()->setAudioPlayer(getAudioPlayer());

    // Spritesheet and other init stuff for entities
    for(auto it = _entityList.begin(); it != _entityList.end(); ++it) {
        it->get()->setPlayerPos(_player->getPos());
        if(it->get()->needsPathToPlayer()) {
            if(it->get()->getEntityType() == EntityType::BISHOP) {
                it->get()->setPathToPlayer(_collisionDetector.diagonalBreadthFirstSearch(it->get()->getPos(), _player->getPos(), _level.get()));
            }
            else {
                it->get()->setPathToPlayer(_collisionDetector.breadthFirstSearch(it->get()->getPos(), _player->getPos(), _level.get()));
            }
        }
        switch(it->get()->getEntityType()) {
            case EntityType::GRUNT:
                it->get()->setSpritesheet(getSpritesheet("GRUNT"));
                it->get()->setShadowSpritesheet(getSpritesheet("SHADOW"));
                break;
            case EntityType::BISHOP:
                it->get()->setSpritesheet(getSpritesheet("BISHOP"));
                it->get()->setShadowSpritesheet(getSpritesheet("SHADOW"));
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
        if(_shoveInAction &&
           !_inMovingState &&
           _player->getHealth() != 0 &&
           _level->getTile(_player->getPos().x, _player->getPos().y).getTileStatus() != TileStatus::BROKEN) {
            _shoveInAction = false;
            if(shovingEntity == nullptr) return;
            _player->setDelta(_player->getPos().x - shovingEntity->getPos().x, _player->getPos().y - shovingEntity->getPos().y);
            SDL_Point oldPos = _player->getPos();
            _collisionDetector.checkForLevelCollisions(_player.get(), _level.get(), true);
            bool pMove = !(oldPos.x == _player->getPos().x && oldPos.y == _player->getPos().y);
            shovingEntity->completePushRequest(pMove);
            if(pMove) {
                _player->setMoveNextMovingState(true);
                startMovingState(DEFAULT_SHOVE_SPEED);
            }
        }
        else if(_spawnEnemy &&
                !_inMovingState) {
            // Spawn enemy
            _spawnEnemy = false;
            addEnemySpawn();
            startMovingState(DEFAULT_SPAWN_SPEED);
        }
        // Post-moving state actions
        else if(!_inMovingState) {
            // Bomb tick
            if(!_bombList.empty()) {
                for(auto b = _bombList.begin(); b != _bombList.end(); ++b) {
                    // This is so hilariously unoptimized but we're not really scaling so YOLO
                    if(b->bombExploded()) {
                        Tile t = _level->getTile(b->getPos().x, b->getPos().y);
                        if(t.canBeDamaged()) t.setTileStatus(TileStatus::BROKEN);
                        t.setTileEvent(TileEvent::NOVAL);
                        _level->setTile(b->getPos().x, b->getPos().y, t);
                        int combo = 1;
                        if(t.isEntityOnTile()) {
                            for(auto it = _entityList.begin(); it != _entityList.end(); ++it) {
                                auto e = it->get();
                                if(e->getPos().x == b->getPos().x && e->getPos().y == b->getPos().y) {
                                    e->hurt(3);
                                    if(e->getHealth() == 0) {
                                        t.setEntityOnTile(false);
                                        _score += BOMB_KILL_SCORE * combo;
                                        ++combo;
                                    }
                                    break;
                                }
                            }
                        }
                        for(auto p : b->getSurroundingPoints()) {
                            t = _level->getTile(p.x, p.y);
                            if(t.canBeDamaged()) t.damageTile(1);
                            if(t.isEntityOnTile()) {
                                for(auto it = _entityList.begin(); it != _entityList.end(); ++it) {
                                    auto e = it->get();
                                    if(e->getPos().x == p.x && e->getPos().y == p.y) {
                                        e->hurt(3);
                                        if(e->getHealth() == 0) {
                                            t.setEntityOnTile(false);
                                            _score += BOMB_KILL_SCORE * combo;
                                            ++combo;
                                        }
                                        break;
                                    }
                                }
                            }
                            _level->setTile(p.x, p.y, t);
                        }
                        b = _bombList.erase(b);
                        --b;
                        getAudioPlayer()->playAudio(_player->getEntityId(), AudioSound::BOMB_BLOW, 1.f);
                    }
                }
            }
            // Shove kill reward
            if(_shoveKills > 0) {
                for(int i = 1; i <= _shoveKills; ++i) {
                    _score += SHOVE_KILL_SCORE * i;
                }
                _shoveKills = 0;
                _player->setRebuildCount(_player->getNumOfRebuilds() + 1);
                _player->setBombCount(_player->getNumOfBombs() + 1);
            }
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
                    if(_level->getTile(_player->getPos().x, _player->getPos().y).getTileStatus() == TileStatus::BROKEN) return;
                    tickBombTimers();
                    ++_moves;
                }
            }
            
            if(_player->requestPush()) {
                _level->addTileEvent(_player.get(), TileEvent::PUSH);
            }
            else if(_player->requestBomb() &&
                    _player->getNumOfBombs() > 0 &&
                    _level->getTile(_player->getPos().x, _player->getPos().y).getTileEvent() != TileEvent::BOMB) {
                _level->addTileEvent(_player.get(), TileEvent::BOMB);
                _player->setBombCount(_player->getNumOfBombs() - 1);
                _bombList.push_back(Bomb(_player->getPos().x, _player->getPos().y, 3));
                _bombList.back().setSpritesheet(getSpritesheet("BOMB"));
                getAudioPlayer()->playAudio(_player->getEntityId(), AudioSound::PLACE_BOMB, 1.f);
            }
            else if(_player->requestRebuild() &&
                    _player->getNumOfRebuilds() > 0 &&
                    _level->getTile(_player->getPosFacing().x, _player->getPosFacing().y).getTileStatus() == TileStatus::BROKEN) {
                _level->addTileEvent(_player.get(), TileEvent::REBUILD);
                _player->setRebuildCount(_player->getNumOfRebuilds() - 1);
                getAudioPlayer()->playAudio(_player->getEntityId(), AudioSound::REBUILD, 0.9f);
                _score += REBUILD_SCORE;
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
                // Entity pushes player
                if(e->requestPush() && e->getPosFacing().x == _player->getPos().x && e->getPosFacing().y == _player->getPos().y) {
                    _shoveInAction = true;
                    shovingEntity = e;
                }
                // Entity moves based on old player path so that they're not insanely hard to evade
                if(e->needsPathToPlayer()) {
                    if(e->getEntityType() == EntityType::BISHOP) {
                        auto path = _collisionDetector.diagonalBreadthFirstSearch(e->getPos(), _player->getPos(), _level.get());
                        e->setPathToPlayer(path);
                    }
                    else {
                        auto path = _collisionDetector.breadthFirstSearch(e->getPos(), _player->getPos(), _level.get());
                        e->setPathToPlayer(path);
                    }
                }
            }
            // Entity event checking
            Tile t = _level->getTile(e->getPos().x, e->getPos().y);
            // Entity gets pushed
            if(t.getTileEvent() == TileEvent::PUSH) {
                int dx = e->getPos().x - _player->getPos().x;
                int dy = e->getPos().y - _player->getPos().y;
                SDL_Point nextPos = {e->getPos().x + dx, e->getPos().y + dy};
                std::vector<Entity*> shovedEntities;
                shovedEntities.push_back(e);
                e->setDelta(dx, dy);
                while(_level->getTile(nextPos.x, nextPos.y).isEntityOnTile()) {
                    // Again horribly unoptimized but we don't got much time
                    for(auto it = _entityList.begin(); it != _entityList.end(); ++it) {
                        Entity* se = it->get();
                        if(se == e) continue;
                        if(se->getPos().x == nextPos.x && se->getPos().y == nextPos.y) {
                            shovedEntities.push_back(se);
                            se->setDelta(dx, dy);
                            Tile t = _level->getTile(se->getPos().x, se->getPos().y);
                            t.setEntityOnTile(false);
                            _level->setTile(se->getPos().x, se->getPos().y, t);
                            nextPos = {se->getPos().x + dx, se->getPos().y + dy};
                        }
                    }
                }
                int combo = 1;
                for(auto se : shovedEntities) {
                    _score += SHOVE_SCORE * combo;
                    ++combo;
                    _collisionDetector.checkForLevelCollisions(se, _level.get(), true);
                    if(se->needsPathToPlayer()) {
                        if(se->getEntityType() == EntityType::BISHOP) {
                            auto path = _collisionDetector.diagonalBreadthFirstSearch(se->getPos(), _player->getPos(), _level.get());
                            se->setPathToPlayer(path);
                        }
                        else {
                            auto path = _collisionDetector.breadthFirstSearch(se->getPos(), _player->getPos(), _level.get());
                            se->setPathToPlayer(path);
                        }
                    }
                    Tile t = _level->getTile(se->getPos().x, se->getPos().y);
                    if(t.getTileStatus() == TileStatus::BROKEN) {
                        se->hurt(99);
                        ++_shoveKills;
                    }
                    se->setMoveNextMovingState(true);
                }
                startMovingState(DEFAULT_SHOVE_SPEED);
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
}

void GameState::render() {
    SDL_SetRenderDrawColor(getRenderer(), 0x76, 0x9a, 0xb3, 0xFF);
    SDL_RenderClear(getRenderer());

    // Level
    _level->render(_renderOffset.x, _renderOffset.y);

    // Player
    if(!_gameOver) {
        _player->render(_level->getRenderPos().x + _renderOffset.x, _level->getRenderPos().y + _renderOffset.y);
    }

    // Entities
    SDL_SetRenderDrawColor(getRenderer(), 0xFF, 0x00, 0x00, 0x64);
    for(auto e = _entityList.begin(); e != _entityList.end(); ++e) {
        e->get()->render(_level->getRenderPos().x + _renderOffset.x, _level->getRenderPos().y + _renderOffset.y);
        // SDL_Rect r = {e->get()->getPosFacing().x * 16 + _level->getRenderPos().x + _renderOffset.x,
        //     e->get()->getPosFacing().y * 16 + _level->getRenderPos().y + _renderOffset.y,
        //     16,
        //     16};
        // SDL_RenderFillRect(getRenderer(), &r);
    }

    for(auto b : _bombList) {
        b.render(_level->getRenderPos().x + _renderOffset.x, _level->getRenderPos().y + _renderOffset.y);
    }

    // HUD
    Text* smallText = getText(TextSize::TINY);
    smallText->setString("bombs: " + std::to_string(_player->getNumOfBombs()));
    smallText->draw(4, 1);
    smallText->setString("rebuilds: " + std::to_string(_player->getNumOfRebuilds()));
    smallText->draw(4, 11);
    std::string score = std::to_string(_score);
    while(score.size() < 7) score.insert(score.begin(), '0');
    smallText->setString("score: " + score);
    smallText->draw(4, 21);
    smallText->setString("moves: " + std::to_string(_moves));
    smallText->draw(4, 31);

    SDL_RenderPresent(getRenderer());
}

void GameState::startMovingState(int moveSpeed) {
    _inMovingState = true;
    _moveTimer = 0;
    _moveSpeed = moveSpeed;
}

void GameState::tickBombTimers() {
    for(auto b = _bombList.begin(); b != _bombList.end(); ++b) {
        b->tickTimer();
    }
}

void GameState::addEnemySpawn() {
    // TODO: enemy spawn stuff
}