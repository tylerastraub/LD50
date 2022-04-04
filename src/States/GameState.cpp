#include "GameState.h"
#include "RandomGen.h"
#include "Grunt.h"
#include "Bishop.h"
#include "Tank.h"

#include <chrono>

/**
 * TODO:
 * - Refine HUD more
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

    // Spritesheet and other init stuff for entities
    for(auto it = _entityList.begin(); it != _entityList.end(); ++it) {
        switch(it->get()->getEntityType()) {
            case EntityType::GRUNT:
                it->get()->setSpritesheet(getSpritesheet("GRUNT"));
                it->get()->setShadowSpritesheet(getSpritesheet("SHADOW"));
                break;
            case EntityType::BISHOP:
                it->get()->setSpritesheet(getSpritesheet("BISHOP"));
                it->get()->setShadowSpritesheet(getSpritesheet("SHADOW"));
                break;
            case EntityType::TANK:
                it->get()->setSpritesheet(getSpritesheet("TANK"));
                it->get()->setShadowSpritesheet(getSpritesheet("SHADOW"));
                break;
        }
    }
}

void GameState::tick(float timescale) {
    // Reset
    if(_keyboard->isKeyPressed(SDL_SCANCODE_R)) setNextState(new GameState());

    // Enemy spawning
    if((_movesSinceLastEnemySpawn >= _currentSpawnWaitTime || _entityList.empty()) &&
        _player->getHealth() > 0) {
        _spawnEnemy = true;
        _inMovingState = true;
    }

    // Move state
    if(_inMovingState) {
        _moveTimer += timescale * _moveSpeed;
        if(_moveTimer >= 1) {
            _inMovingState = false;
            _moveTimer = 1.f;
        }
        // Move player
        if(_player->moveNextMovingState()) {
            int playerDx = _player->getPos().x - _player->getLastPos().x;
            int playerDy = _player->getPos().y - _player->getLastPos().y;
            _player->setRenderPos((_player->getLastPos().x + _moveTimer * playerDx) * _level->getTileSize(),
                (_player->getLastPos().y + _moveTimer * playerDy) * _level->getTileSize());
            _player->setMoveNextMovingState(_inMovingState);
        }
        // Move entities
        for(auto it = _entityList.begin(); it != _entityList.end(); ++it) {
            Entity* e = it->get();
            if(e->moveNextMovingState()) {
                int eDx = e->getPos().x - e->getLastPos().x;
                int eDy = e->getPos().y - e->getLastPos().y;
                e->setRenderPos((e->getLastPos().x + _moveTimer * eDx) * _level->getTileSize(),
                    (e->getLastPos().y + _moveTimer * eDy) * _level->getTileSize());
                e->setMoveNextMovingState(_inMovingState);
                if(e->isSpawning()) e->setSpawning(_inMovingState);
            }
        }
        // Player got shoved
        if(_shoveInAction &&
           !_inMovingState &&
           _player->getHealth() != 0 &&
           _level->getTile(_player->getPos().x, _player->getPos().y).getTileStatus() != TileStatus::BROKEN) {
            _shoveInAction = false;
            if(shovingEntity == nullptr) return;
            int dx = _player->getPos().x - shovingEntity->getPos().x;
            int dy = _player->getPos().y - shovingEntity->getPos().y;
            _player->setDelta(dx * (shovingEntity->getWeight() + 1), dy * (shovingEntity->getWeight() + 1));
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
                getAudioPlayer()->playAudio(_player->getEntityId(), AudioSound::ITEM_EARNED, 0.9f);
            }
        }
    }
    else {
        bool playerMove = false;
        // Player tick
        if(_player->getHealth() == 0) {
            if((_level->getTile(_player->getPos().x, _player->getPos().y).getTileStatus() == TileStatus::BROKEN ||
               _level->getTile(_player->getPos().x, _player->getPos().y).getTileStatus() == TileStatus::NOVAL) &&
               !_gameOver) {
                getAudioPlayer()->playAudio(_player->getEntityId(), AudioSound::FALL, 1.f);
            }
            _gameOver = true;
            _keyboard->updateInputs();
            return;
        }
        else {
            if(_level->getTile(_player->getPos().x, _player->getPos().y).getTileStatus() == TileStatus::BROKEN ||
               _level->getTile(_player->getPos().x, _player->getPos().y).getTileStatus() == TileStatus::NOVAL) {
                _player->hurt(99);
            }
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
                    ++_movesSinceLastEnemySpawn;
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
            if(_level->getTile(e->getPos().x, e->getPos().y).getTileStatus() == TileStatus::BROKEN ||
               _level->getTile(e->getPos().x, e->getPos().y).getTileStatus() == TileStatus::NOVAL) {
                e->hurt(99);
            }
            // Entity removal
            if(e->getHealth() == 0) {
                if(_level->getTile(e->getPos().x, e->getPos().y).getTileStatus() == TileStatus::BROKEN ||
                   _level->getTile(e->getPos().x, e->getPos().y).getTileStatus() == TileStatus::NOVAL) {
                    getAudioPlayer()->playAudio(e->getEntityId(), AudioSound::FALL, 1.f);
                }
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
                int loopCount = 0;
                // Don't have time to figure out why this sometimes hangs, 500 iteration limit will have to do
                while(_level->getTile(nextPos.x, nextPos.y).isEntityOnTile() && loopCount < 500) {
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
                    ++loopCount;
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
                    if(t.getTileStatus() == TileStatus::BROKEN || t.getTileStatus() == TileStatus::NOVAL) {
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

    if(_gameOver) {
        Text* medText = getText(TextSize::MEDIUM);
        medText->setString("GAME OVER");
        medText->draw(getGameSize().x / 2 - medText->getWidth() / 2, 20);
        smallText->setString("press 'R' to restart");
        smallText->draw(getGameSize().x / 2 - smallText->getWidth() / 2 - 3, 40);
        smallText->setString("press 'ESC' to quit");
        smallText->draw(getGameSize().x / 2 - smallText->getWidth() / 2 - 3, 50);
    }

    SDL_RenderPresent(getRenderer());
}

void GameState::startMovingState(float moveSpeed) {
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
    _movesSinceLastEnemySpawn = 0;
    _currentSpawnWaitTime = RandomGen::getRandomInt(MIN_MOVES_BETWEEN_SPAWNS, MAX_MOVES_BETWEEN_SPAWNS);
    int spawnIndex = RandomGen::getRandomInt(0, _level->getValidSpawnTiles().size() - 1);
    SDL_Point spawnPoint = {spawnIndex % _level->getHeight(), spawnIndex / _level->getHeight()};
    int randomCount = 0;
    while(_level->getTile(spawnPoint.x, spawnPoint.y).isEntityOnTile()) {
        int spawnIndex = RandomGen::getRandomInt(0, _level->getValidSpawnTiles().size() - 1);
        SDL_Point spawnPoint = {spawnIndex % _level->getHeight(), spawnIndex / _level->getHeight()};
        ++randomCount;
        // safeguard cause there's a crash here i can't pinpoint in time
        if(randomCount > 500) break;
    }
    Tile t = _level->getTile(spawnPoint.x, spawnPoint.y);
    t.setEntityOnTile(true);
    _level->setTile(spawnPoint.x, spawnPoint.y, t);

    // This can be a lot cleaner (especially with RDS tables) but again... no time!
    int totalProb = GRUNT_SPAWN_CHANCE + BISHOP_SPAWN_CHANCE + TANK_SPAWN_CHANCE;
    int probHit = RandomGen::getRandomInt(1, totalProb);
    if(probHit <= GRUNT_SPAWN_CHANCE || _moves == 0) {
        _entityList.push_back(std::make_unique<Grunt>());
        _entityList.back()->setPos(spawnPoint.x, spawnPoint.y);
        _entityList.back()->setLastPos(spawnPoint.x, spawnPoint.y - 20);
        _entityList.back()->setRenderPos(spawnPoint.x * _level->getTileSize(), spawnPoint.y * _level->getTileSize() - 320);
        _entityList.back()->setAudioPlayer(getAudioPlayer());
        _entityList.back()->setSpritesheet(getSpritesheet("GRUNT"));
        _entityList.back()->setShadowSpritesheet(getSpritesheet("SHADOW"));
        _entityList.back()->setPlayerPos(_player->getPos());
        _entityList.back()->setPathToPlayer(_collisionDetector.breadthFirstSearch(_entityList.back()->getPos(), _player->getPos(), _level.get()));
        _entityList.back()->setMoveNextMovingState(true);
    }
    else if(probHit <= GRUNT_SPAWN_CHANCE + BISHOP_SPAWN_CHANCE) {
        _entityList.push_back(std::make_unique<Bishop>());
        _entityList.back()->setPos(spawnPoint.x, spawnPoint.y);
        _entityList.back()->setLastPos(spawnPoint.x, spawnPoint.y - 20);
        _entityList.back()->setRenderPos(spawnPoint.x * _level->getTileSize(), spawnPoint.y * _level->getTileSize() - 320);
        _entityList.back()->setAudioPlayer(getAudioPlayer());
        _entityList.back()->setSpritesheet(getSpritesheet("BISHOP"));
        _entityList.back()->setShadowSpritesheet(getSpritesheet("SHADOW"));
        _entityList.back()->setPathToPlayer(_collisionDetector.diagonalBreadthFirstSearch(_entityList.back()->getPos(), _player->getPos(), _level.get()));
        _entityList.back()->setMoveNextMovingState(true);
    }
    else {
        _entityList.push_back(std::make_unique<Tank>());
        _entityList.back()->setPos(spawnPoint.x, spawnPoint.y);
        _entityList.back()->setLastPos(spawnPoint.x, spawnPoint.y - 20);
        _entityList.back()->setRenderPos(spawnPoint.x * _level->getTileSize(), spawnPoint.y * _level->getTileSize() - 320);
        _entityList.back()->setAudioPlayer(getAudioPlayer());
        _entityList.back()->setSpritesheet(getSpritesheet("TANK"));
        _entityList.back()->setShadowSpritesheet(getSpritesheet("SHADOW"));
        _entityList.back()->setPathToPlayer(_collisionDetector.breadthFirstSearch(_entityList.back()->getPos(), _player->getPos(), _level.get()));
        _entityList.back()->setMoveNextMovingState(true);
    }
    getAudioPlayer()->playAudio(_entityList.back()->getEntityId(), AudioSound::SPAWN, 0.8f);
}