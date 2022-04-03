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
    _player->setSpritesheet(getSpritesheet("PLAYER"));
    _player->setKeyboard(_keyboard.get());
    _player->setAudioPlayer(getAudioPlayer());

    // Entites
    _entityList.push_back(std::make_unique<Grunt>());
    _entityList.back()->setPos(7, 8);
    _entityList.back()->setAudioPlayer(getAudioPlayer());

    // Spritesheet and other init stuff for entities
    for(auto it = _entityList.begin(); it != _entityList.end(); ++it) {
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
    if(_keyboard->isKeyPressed(SDL_SCANCODE_R)) setNextState(new GameState());
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
            playerMove = !(oldPos.x == _player->getPos().x && oldPos.y == _player->getPos().y);
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
    for(auto it = _entityList.begin(); it != _entityList.end(); ++it) {
        Entity* e = it->get();
        e->tick(timescale);
        if(playerMove) {
            e->onPlayerAction();
            if(e->isMoving()) {
                _collisionDetector.checkForLevelCollisions(e, _level.get());
            }
            // Entity moves based on old player path so that they're not insanely hard to evade
            if(e->needsPathToPlayer()) {
                auto path = _collisionDetector.breadthFirstSearch(e->getPos(), _player->getPos(), _level.get());
                e->setPathToPlayer(path);
            }
        }
        Tile t = _level->getTile(e->getPos().x, e->getPos().y);
        // Event checking
        if(t.getTileEvent() == TileEvent::PUSH) {
            e->setDelta(e->getPos().x - _player->getPos().x, e->getPos().y - _player->getPos().y);
            _collisionDetector.checkForLevelCollisions(e, _level.get());
            if(e->needsPathToPlayer()) {
                auto path = _collisionDetector.breadthFirstSearch(e->getPos(), _player->getPos(), _level.get());
                e->setPathToPlayer(path);
            }
        }
        
        if(e->getHealth() == 0) {
            it = _entityList.erase(it);
            --it;
            continue;
        }
    }

    // Level update
    if(_player->requestPush()) {
        // Since by now the push should have been processed, we can remove the push event
        Tile t = _level->getTile(_player->getPosFacing().x, _player->getPosFacing().y);
        if(t.getTileEvent() == TileEvent::PUSH) t.setTileEvent(TileEvent::NOVAL);
        _level->setTile(_player->getPosFacing().x, _player->getPosFacing().y, t);
    }

    // Input updates
    _keyboard->updateInputs();
    _controller->updateInputs();
}

void GameState::render() {
    SDL_SetRenderDrawColor(getRenderer(), 0x76, 0x9a, 0xb3, 0xFF);
    SDL_RenderClear(getRenderer());

    _level->render(_renderOffset.x, _renderOffset.y);
    if(_player->getHealth() > 0) {
        _player->render(_level->getRenderPos().x + _renderOffset.x, _level->getRenderPos().y + _renderOffset.y);
    }

    for(auto e = _entityList.begin(); e != _entityList.end(); ++e) {
        e->get()->render(_level->getRenderPos().x + _renderOffset.x,
            _level->getRenderPos().y + _renderOffset.y);
    }

    SDL_RenderPresent(getRenderer());
}

void GameState::handleControllerButtonInput(SDL_Event e) {
    _controller->updateButtonInputs(e);
}

void GameState::handleControllerAxisInput(SDL_Event e) {
    _controller->updateAxisInputs(e);
}