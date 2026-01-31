#ifndef HERO_H
#define HERO_H
#include <vector>
#include <set>
#include <string>

class Maze;

class Hero {
private:
    int x, y;
    char symbol;
    std::string name;
    bool hasKey;
    bool isTrapped;
    
    // Memory system
    std::vector<std::vector<bool>> visited;
    std::vector<std::vector<char>> knownMap;
    int mapWidth, mapHeight; 
    std::pair<int, int> lastMove;
    std::pair<int, int> previousPosition;
    int stuckCounter;  // Counter for stucks

    std::vector<std::pair<int, int>> blockedPositions;
    
    std::vector<std::pair<int, int>> getValidMoves(const Maze* maze) const;
    std::pair<int, int> exploreUnknown(const Maze* maze);
    std::pair<int, int> exploreUnknownSmart(const Maze* maze);
    std::pair<int, int> exploreUnknownSmartWithBlocked(const Maze* maze);
    std::pair<int, int> moveTowardsTarget(int targetX, int targetY, const Maze* maze);
    std::pair<int, int> randomValidMove(const Maze* maze);
    std::pair<int, int> smartRandomMove(const Maze* maze);
    std::pair<int, int> smartRandomMoveWithBlocked(const Maze* maze);
    
    // Movement memory
    void updateMovementMemory(int newX, int newY);
    bool isRepeatingMove(int targetX, int targetY) const;
    bool isBlockedPosition(int x, int y) const;
    
public:
    Hero(int startX, int startY, char sym, const std::string& heroName, int mWidth, int mHeight);
    ~Hero();
    
    // Position and state
    int getX() const { return x; }
    int getY() const { return y; }
    char getSymbol() const { return symbol; }
    std::string getName() const { return name; }
    bool getHasKey() const { return hasKey; }
    bool getIsTrapped() const { return isTrapped; }
    
    void setPosition(int newX, int newY);
    void setHasKey(bool key);
    void setTrapped(bool trapped) { isTrapped = trapped; }
    
    // Memory management
    void updateVision(const Maze* maze);
    void markVisited(int posX, int posY);
    bool hasVisited(int posX, int posY) const;
    char getKnownCell(int posX, int posY) const;

    void notifyBlockedMove(int blockedX, int blockedY);
    void clearBlockedPositions(); 

    std::pair<int, int> decideNextMove(const Maze* maze, int keyX = -1, int keyY = -1, 
                                       const std::vector<std::pair<int, int>>& visibleCages = {});
    
    bool isAdjacent(int otherX, int otherY) const;
    bool canSeePosition(int targetX, int targetY) const;
};

#endif