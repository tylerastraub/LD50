#include "CollisionDetector.h"

#include <set>
#include <algorithm>

void CollisionDetector::checkForLevelCollisions(Entity* e, Level* level, bool wasPushed) {
    SDL_Point eDelta = e->getDelta();
    SDL_Point ePos = e->getPos();
    SDL_Point eNewPos = {ePos.x + eDelta.x, ePos.y + eDelta.y};

    // First we move (if possible)
    if(eDelta.x != 0 || eDelta.y != 0) {
        Tile t = level->getTile(eNewPos.x, eNewPos.y);
        if(t.getTileType() != TileType::NOVAL && t.getTileType() != TileType::WALL && !t.isEntityOnTile()) {
            e->setPos(eNewPos.x, eNewPos.y);
            level->onTileMovedFrom(ePos.x, ePos.y, e, wasPushed);
            level->onTileMovedTo(eNewPos.x, eNewPos.y, e, wasPushed);
            e->onMove();
            Tile t = level->getTile(eNewPos.x, eNewPos.y);
            t.setEntityOnTile(true);
            level->setTile(eNewPos.x, eNewPos.y, t);
            t = level->getTile(ePos.x, ePos.y);
            t.setEntityOnTile(false);
            level->setTile(ePos.x, ePos.y, t);
        }
        else if(t.getTileType() == TileType::NOVAL) {
            e->setPos(eNewPos.x, eNewPos.y);
            level->onTileMovedFrom(ePos.x, ePos.y, e, wasPushed);
            e->onMove();
            Tile t = level->getTile(ePos.x, ePos.y);
            t.setEntityOnTile(false);
            level->setTile(ePos.x, ePos.y, t);
            e->hurt(99);
        }
        // This is not tested yet
        else if(std::abs(eDelta.x) > 1 || std::abs(eDelta.y) > 1) {
            int xShrink = 0;
            int yShrink = 0;
            if(eDelta.x != 0) xShrink = (eDelta.x < 0) ? 1 : -1;
            if(eDelta.y != 0) yShrink = (eDelta.y < 0) ? 1 : -1;
            e->setDelta(eDelta.x + xShrink, eDelta.y + yShrink);
            checkForLevelCollisions(e, level);
        }
        e->stop();
    }

    // Then we check for any level events
    if(level->getTile(eNewPos.x, eNewPos.y).getTileStatus() == TileStatus::BROKEN) {
        e->hurt(99);
        Tile t = level->getTile(eNewPos.x, eNewPos.y);
        t.setEntityOnTile(false);
        level->setTile(eNewPos.x, eNewPos.y, t);
    }
}

std::vector<SDL_Point> CollisionDetector::breadthFirstSearch(SDL_Point start, SDL_Point goal, Level* level) {
    std::queue<Node> queue;
    std::vector<Node> visited;
    std::vector<SDL_Point> result;
    SDL_Point offsets[4] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    queue.push({start, start, 0});
    visited.push_back({start, start, 0});

    if(start.x == goal.x && start.y == goal.y) return std::vector<SDL_Point>();
    while(!queue.empty()) {
        Node currentPoint = queue.front();
        queue.pop();
        for(auto offset : offsets) {
            Node nextNode = {
                {currentPoint.pos.x + offset.x, currentPoint.pos.y + offset.y},
                currentPoint.pos,
                currentPoint.cost + 1};
            Tile t = level->getTile(nextNode.pos.x, nextNode.pos.y);
            if(std::find(visited.begin(), visited.end(), nextNode) == visited.end() &&
               t.getTileType() != TileType::NOVAL &&
               t.getTileType() != TileType::WALL &&
               t.getTileStatus() != TileStatus::BROKEN) {
                visited.push_back(nextNode);
                if(nextNode.pos.x == goal.x && nextNode.pos.y == goal.y) {
                    while(nextNode.pos.x != start.x || nextNode.pos.y != start.y) {
                        result.push_back(nextNode.pos);
                        Node searchNode = {nextNode.parent, {}, 0};
                        auto it = std::find(visited.begin(), visited.end(), searchNode);
                        if(it == visited.end()) return std::vector<SDL_Point>();
                        nextNode = *it;
                    }
                    std::reverse(result.begin(), result.end());
                    return result;
                }
                else {
                    queue.push(nextNode);
                }
            }
        }
    }
    // return empty queue - we didn't find a path
    return std::vector<SDL_Point>();
}

// We probably don't need another entire method for this but whatever
std::vector<SDL_Point> CollisionDetector::diagonalBreadthFirstSearch(SDL_Point start, SDL_Point goal, Level* level) {
    std::queue<Node> queue;
    std::vector<Node> visited;
    std::vector<SDL_Point> result;
    SDL_Point offsets[4] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    // Since sometimes it is impossible to reach a tile diagonally, we stop if we get close enough
    SDL_Point goalOffsets[8] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}, {1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    queue.push({start, start, 0});
    visited.push_back({start, start, 0});

    if(start.x == goal.x && start.y == goal.y) return std::vector<SDL_Point>();
    while(!queue.empty()) {
        Node currentPoint = queue.front();
        queue.pop();
        for(auto offset : offsets) {
            Node nextNode = {
                {currentPoint.pos.x + offset.x, currentPoint.pos.y + offset.y},
                currentPoint.pos,
                currentPoint.cost + 1};
            Tile t = level->getTile(nextNode.pos.x, nextNode.pos.y);
            if(std::find(visited.begin(), visited.end(), nextNode) == visited.end() &&
               t.getTileType() != TileType::NOVAL &&
               t.getTileType() != TileType::WALL &&
               t.getTileStatus() != TileStatus::BROKEN) {
                visited.push_back(nextNode);
                if(nextNode.pos.x == goal.x && nextNode.pos.y == goal.y) {
                    while(nextNode.pos.x != start.x || nextNode.pos.y != start.y) {
                        result.push_back(nextNode.pos);
                        Node searchNode = {nextNode.parent, {}, 0};
                        auto it = std::find(visited.begin(), visited.end(), searchNode);
                        if(it == visited.end()) return std::vector<SDL_Point>();
                        nextNode = *it;
                    }
                    std::reverse(result.begin(), result.end());
                    return result;
                }
                else {
                    for(auto p : goalOffsets) {
                        if(nextNode.pos.x + p.x == goal.x && nextNode.pos.y + p.y == goal.y) {
                            while(nextNode.pos.x != start.x || nextNode.pos.y != start.y) {
                                result.push_back(nextNode.pos);
                                Node searchNode = {nextNode.parent, {}, 0};
                                auto it = std::find(visited.begin(), visited.end(), searchNode);
                                if(it == visited.end()) return std::vector<SDL_Point>();
                                nextNode = *it;
                            }
                            std::reverse(result.begin(), result.end());
                            return result;
                        }
                    }
                    queue.push(nextNode);
                }
            }
        }
    }
    // return empty queue - we didn't find a path
    return std::vector<SDL_Point>();
}