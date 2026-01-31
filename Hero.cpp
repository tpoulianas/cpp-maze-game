#include "Hero.h"
#include "Maze.h"
#include <algorithm>
#include <random>
#include <cstdlib>
#include <cmath>

using namespace std;

Hero::Hero(int startX, int startY, char sym, const string& heroName, int mWidth, int mHeight) 
    : x(startX), y(startY), symbol(sym), name(heroName), hasKey(false), isTrapped(false),
      mapWidth(mWidth), mapHeight(mHeight), lastMove({0, 0}), 
      previousPosition({startX, startY}), stuckCounter(0) {
    
    // Initialize memory systems
    visited.resize(mapHeight);
    knownMap.resize(mapHeight);
    for (int i = 0; i < mapHeight; i++) {
        visited[i].resize(mapWidth, false);
        knownMap[i].resize(mapWidth, '?'); // Unknown areas
    }
}

Hero::~Hero() {
    // Vectors automatically clean up
}

void Hero::setPosition(int newX, int newY) {
    updateMovementMemory(newX, newY);
    x = newX;
    y = newY;
    markVisited(x, y);
}

void Hero::setHasKey(bool key) { 
    hasKey = key; 
    if (key) {
        clearBlockedPositions(); 
    }
}

void Hero::updateMovementMemory(int newX, int newY) {
    // Check for stuck in the same place 
    if (newX == previousPosition.first && newY == previousPosition.second) {
        stuckCounter++;
    } else {
        stuckCounter = 0;
        lastMove = {newX - x, newY - y};
        previousPosition = {x, y};
    }
}

bool Hero::isRepeatingMove(int targetX, int targetY) const {
    if (stuckCounter < 2) return false;  
    
    int moveX = targetX - x;
    int moveY = targetY - y;
    
    // Check if the movement is the same as before
    return (moveX == lastMove.first && moveY == lastMove.second);
}

void Hero::notifyBlockedMove(int blockedX, int blockedY) {
    for (const auto& pos : blockedPositions) {
        if (pos.first == blockedX && pos.second == blockedY) {
            return; 
        }
    }
    blockedPositions.push_back({blockedX, blockedY});
}

void Hero::clearBlockedPositions() {
    blockedPositions.clear();
}

bool Hero::isBlockedPosition(int x, int y) const {
    for (const auto& pos : blockedPositions) {
        if (pos.first == x && pos.second == y) {
            return true;
        }
    }
    return false;
}

void Hero::updateVision(const Maze* maze) {
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int checkX = x + dx;
            int checkY = y + dy;
            
            if (maze->isValidPosition(checkX, checkY)) {
                knownMap[checkY][checkX] = maze->getCell(checkX, checkY);
            }
        }
    }
}

void Hero::markVisited(int posX, int posY) {
    if (posX >= 0 && posX < mapWidth && posY >= 0 && posY < mapHeight) {
        visited[posY][posX] = true;
    }
}

bool Hero::hasVisited(int posX, int posY) const {
    if (posX >= 0 && posX < mapWidth && posY >= 0 && posY < mapHeight) {
        return visited[posY][posX];
    }
    return false;
}

char Hero::getKnownCell(int posX, int posY) const {
    if (posX >= 0 && posX < mapWidth && posY >= 0 && posY < mapHeight) {
        return knownMap[posY][posX];
    }
    return '*'; // Assume wall if out of bounds
}

bool Hero::canSeePosition(int targetX, int targetY) const {
    return abs(targetX - x) <= 1 && abs(targetY - y) <= 1;
}

vector<pair<int, int>> Hero::getValidMoves(const Maze* maze) const {
    vector<pair<int, int>> moves;
    
    // Check all 4 directions
    int dx[] = {0, 1, 0, -1};
    int dy[] = {-1, 0, 1, 0};
    
    for (int i = 0; i < 4; i++) {
        int newX = x + dx[i];
        int newY = y + dy[i];
        
        if (maze->isValidPosition(newX, newY) && !maze->isWall(newX, newY)) {
            moves.push_back({newX, newY});
        }
    }
    
    return moves;
}

pair<int, int> Hero::exploreUnknown(const Maze* maze) {
    vector<pair<int, int>> validMoves = getValidMoves(maze);
    vector<pair<int, int>> unexploredMoves;
    
    // Prefer unvisited positions
    for (const auto& move : validMoves) {
        if (!hasVisited(move.first, move.second)) {
            unexploredMoves.push_back(move);
        }
    }
    
    if (!unexploredMoves.empty()) {
        return unexploredMoves[rand() % unexploredMoves.size()];
    }
    
    // If all nearby positions visited, try to find path to unexplored areas
    return randomValidMove(maze);
}

pair<int, int> Hero::exploreUnknownSmart(const Maze* maze) {
    vector<pair<int, int>> validMoves = getValidMoves(maze);
    vector<pair<int, int>> unexploredMoves;
    vector<pair<int, int>> nonRepeatingMoves;
    
    // Find movements that don't repeat previous movements
    for (const auto& move : validMoves) {
        if (!isRepeatingMove(move.first, move.second)) {
            nonRepeatingMoves.push_back(move);
            
            if (!hasVisited(move.first, move.second)) {
                unexploredMoves.push_back(move);
            }
        }
    }
    
    if (!unexploredMoves.empty()) {
        return unexploredMoves[rand() % unexploredMoves.size()];
    }

    if (!nonRepeatingMoves.empty()) {
        return nonRepeatingMoves[rand() % nonRepeatingMoves.size()];
    }

    if (!validMoves.empty()) {
        return validMoves[rand() % validMoves.size()];
    }
    
    return {x, y};
}

pair<int, int> Hero::exploreUnknownSmartWithBlocked(const Maze* maze) {
    vector<pair<int, int>> validMoves = getValidMoves(maze);
    vector<pair<int, int>> unexploredMoves;
    vector<pair<int, int>> nonRepeatingMoves;

    for (const auto& move : validMoves) {
        if (!isRepeatingMove(move.first, move.second) && 
            !isBlockedPosition(move.first, move.second)) {
            nonRepeatingMoves.push_back(move);
            
            if (!hasVisited(move.first, move.second)) {
                unexploredMoves.push_back(move);
            }
        }
    }
    
    // Priority to Unexplored areas
    if (!unexploredMoves.empty()) {
        return unexploredMoves[rand() % unexploredMoves.size()];
    }
    
    // If all moves have been explored, select from the non-repeating ones 
    if (!nonRepeatingMoves.empty()) {
        return nonRepeatingMoves[rand() % nonRepeatingMoves.size()];
    }
    
    // If all repeat or are blocked, choose randomly from the valid ones
    if (!validMoves.empty()) {
        return validMoves[rand() % validMoves.size()];
    }
    
    return {x, y}; 
}

pair<int, int> Hero::moveTowardsTarget(int targetX, int targetY, const Maze* maze) {
    vector<pair<int, int>> validMoves = getValidMoves(maze);
    
    if (validMoves.empty()) {
        return {x, y}; 
    }
    
    // Find move that gets closest to target
    pair<int, int> bestMove = validMoves[0];
    int bestDistance = abs(bestMove.first - targetX) + abs(bestMove.second - targetY);
    
    for (const auto& move : validMoves) {
        int distance = abs(move.first - targetX) + abs(move.second - targetY);
        if (distance < bestDistance) {
            bestDistance = distance;
            bestMove = move;
        }
    }
    
    return bestMove;
}

pair<int, int> Hero::randomValidMove(const Maze* maze) {
    vector<pair<int, int>> validMoves = getValidMoves(maze);
    
    if (validMoves.empty()) {
        return {x, y}; 
    }
    
    return validMoves[rand() % validMoves.size()];
}

pair<int, int> Hero::smartRandomMove(const Maze* maze) {
    vector<pair<int, int>> validMoves = getValidMoves(maze);
    vector<pair<int, int>> smartMoves;
    
    for (const auto& move : validMoves) {
        if (!isRepeatingMove(move.first, move.second)) {
            smartMoves.push_back(move);
        }
    }
    
    if (!smartMoves.empty()) {
        return smartMoves[rand() % smartMoves.size()];
    }
    
    if (!validMoves.empty()) {
        return validMoves[rand() % validMoves.size()];
    }
    
    return {x, y}; 
}

pair<int, int> Hero::smartRandomMoveWithBlocked(const Maze* maze) {
    vector<pair<int, int>> validMoves = getValidMoves(maze);
    vector<pair<int, int>> smartMoves;
    
    for (const auto& move : validMoves) {
        if (!isRepeatingMove(move.first, move.second) && 
            !isBlockedPosition(move.first, move.second)) {
            smartMoves.push_back(move);
        }
    }
    
    if (!smartMoves.empty()) {
        return smartMoves[rand() % smartMoves.size()];
    }

    if (!validMoves.empty()) {
        return validMoves[rand() % validMoves.size()];
    }
    
    return {x, y}; 
}

pair<int, int> Hero::decideNextMove(const Maze* maze, int keyX, int keyY, 
                                         const vector<pair<int, int>>& visibleCages) {
    if (isTrapped) {
        return {x, y}; // Can't move when trapped
    }
    
    // Priority 1: Move towards visible key if we don't have it
    if (!hasKey && keyX >= 0 && keyY >= 0 && canSeePosition(keyX, keyY)) {
        pair<int, int> keyMove = moveTowardsTarget(keyX, keyY, maze);
        if (!isRepeatingMove(keyMove.first, keyMove.second) && 
            !isBlockedPosition(keyMove.first, keyMove.second)) {
            return keyMove;
        }
    }

    if (hasKey && !visibleCages.empty()) {
        int cageX = visibleCages[0].first;
        int cageY = visibleCages[0].second;
        pair<int, int> cageMove = moveTowardsTarget(cageX, cageY, maze);
        if (!isRepeatingMove(cageMove.first, cageMove.second)) {
            return cageMove;
        }
    }
    
    // Priority 2: Explore unknown areas 
    pair<int, int> exploreMove = exploreUnknownSmartWithBlocked(maze);
    if (exploreMove.first != x || exploreMove.second != y) {
        return exploreMove;
    }
    
    // Priority 3: Smart random movement
    return smartRandomMoveWithBlocked(maze);
}

bool Hero::isAdjacent(int otherX, int otherY) const {
    return abs(x - otherX) <= 1 && abs(y - otherY) <= 1;
}