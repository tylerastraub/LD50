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
     * @brief Calculates a path between two points using the BFS. Only allows for movement on one axis at a time.
     * 
     * @param start The start point
     * @param target The target point
     * @param level The level/graph to be navigated
     * @param avoidHazards Whether or not the search should ignore broken tiles
     * @return std::vector<SDL_Point> Vector of points containing the path (in order) from start to target
     */
    std::vector<SDL_Point> breadthFirstSearch(SDL_Point start, SDL_Point goal, Level* level, bool avoidHazards = false);

    /**
     * @brief Calculates a path between two points using the BFS. Only allows for movement diagonally.
     * 
     * @param start The start point
     * @param target The target point
     * @param level The level/graph to be navigated
     * @param avoidHazards Whether or not the search should ignore broken tiles
     * @return std::vector<SDL_Point> Vector of points containing the path (in order) from start to target
     */
    std::vector<SDL_Point> diagonalBreadthFirstSearch(SDL_Point start, SDL_Point goal, Level* level, bool avoidHazards = false);

private:

};

#endif