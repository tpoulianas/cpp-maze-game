#ifndef GAME_H
#define GAME_H

#include <vector>
#include <string>
#include <ncurses.h>
#include "Maze.h"
#include "Hero.h"
#include "GameObject.h"

class Game {
private:
    Maze* maze;
    Hero* gregorakis;
    Hero* asimenia;
    GameObject* trap1;
    GameObject* trap2;
    GameObject* cage1;
    GameObject* cage2;
    GameObject* key;
    GameObject* ladder;
    
    int turns;
    bool gameWon;
    bool gameLost;
    bool heroesFound;
    bool wallsDisappearing;
    int wallDisappearCounter;
    
    // Μεταβλητές για τη φάση μετακίνησης προς σκάλα
    bool movingToLadder;
    std::vector<std::pair<int, int>> wallsToRemove;
    
    void initializeGame(const std::string& mapFile);
    void placeObjectsRandomly();
    void updateDisplay();
    void processHeroTurn(Hero* hero);
    void checkGameConditions();
    void checkCollisions(Hero* hero);
    void startWallDisappearing();
    void updateWallDisappearing();
    void moveHeroesToLadder();
    void startMovingToLadder();
    
    bool isCagePosition(int x, int y) const;
    
    bool isValidPosition(int x, int y);
    bool isPositionOccupied(int x, int y);
    int manhattanDistance(int x1, int y1, int x2, int y2);
    
public:
    Game(const std::string& mapFile);
    ~Game();
    
    void run();
    bool isGameOver() const;
    bool isGameWon() const;
};

#endif