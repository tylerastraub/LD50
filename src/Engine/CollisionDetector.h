#ifndef COLLISION_DETECTOR_H
#define COLLISION_DETECTOR_H

#include "Entity.h"
#include "Level.h"

#include <queue>

struct Node {
    SDL_Point pos;
    SDL_Point parent;
    int cost;

    bool operator<(const Node& rhs) {
        return cost < rhs.cost;
    }

    bool operator==(const Node& rhs) {
        return pos.x == rhs.pos.x && pos.y == rhs.pos.y;
    }
};

class CollisionDetector {
public:
    CollisionDetector() = default;
    ~CollisionDetector() = default;

    void checkForLevelCollisions(Entity* e, Level* level);

    /**
     * @brief Calculates a path between two points using the A* pathfinding algorithm
     * 
     * @param start The start point
     * @param target The target point
     * @param level The level/graph to be navigated
     * @return std::vector<Node> Vector of points containing the path (in order) from start to target
     */
    std::priority_queue<Node> aStarPathfind(SDL_Point start, SDL_Point goal, Level* level);
    std::vector<SDL_Point> breadthFirstSearch(SDL_Point start, SDL_Point goal, Level* level);

private:

};

#endif