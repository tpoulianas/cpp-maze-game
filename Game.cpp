#include "Game.h"
#include <iostream>
#include <random>
#include <algorithm>
#include <unistd.h>

using namespace std;

Game::Game(const string& mapFile) 
    : maze(nullptr), gregorakis(nullptr), asimenia(nullptr), 
      trap1(nullptr), trap2(nullptr), cage1(nullptr), cage2(nullptr),
      key(nullptr), ladder(nullptr), turns(0), gameWon(false), gameLost(false),
      heroesFound(false), wallsDisappearing(false), wallDisappearCounter(0),
      movingToLadder(false) {
    
    initializeGame(mapFile);
}

Game::~Game() {
    delete maze;
    delete gregorakis;
    delete asimenia;
    delete trap1;
    delete trap2;
    delete cage1;
    delete cage2;
    delete key;
    delete ladder;
    
    endwin(); // Clean up ncurses
}

void Game::initializeGame(const string& mapFile) {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    curs_set(0); 
    
    // Initialize colors
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK);    // Heroes
        init_pair(2, COLOR_YELLOW, COLOR_BLACK); // Key
        init_pair(3, COLOR_GREEN, COLOR_BLACK);  // Ladder
        init_pair(4, COLOR_MAGENTA, COLOR_BLACK); // Cages
        init_pair(5, COLOR_CYAN, COLOR_BLACK);   // Info text
    }
    
    // Load maze
    maze = new Maze(mapFile);
    
    // Create ladder object at maze's ladder position
    ladder = new GameObject(maze->getLadderX(), maze->getLadderY(), 'L', ObjectType::LADDER);
    
    // Place objects randomly
    placeObjectsRandomly();
    
    // Initialize heroes' memory with maze dimensions
    gregorakis->updateVision(maze);
    asimenia->updateVision(maze);
}

void Game::placeObjectsRandomly() {
    vector<pair<int, int>> freePositions;
    
    // Find all free positions inside the maze (not on borders)
    for (int y = 1; y < maze->getHeight() - 1; y++) {  
        for (int x = 1; x < maze->getWidth() - 1; x++) {   
            if (!maze->isWall(x, y) && 
                !(x == maze->getLadderX() && y == maze->getLadderY())) {
                freePositions.push_back({x, y});
            }
        }
    }
    
    if (freePositions.size() < 5) {
        throw runtime_error("Not enough free positions in maze");
    }
    
    // Shuffle positions
    shuffle(freePositions.begin(), freePositions.end(), default_random_engine{});
    
    // Place heroes with minimum distance of 7
    bool validPlacement = false;
    int attempts = 0;
    while (!validPlacement && attempts < 1000) {
        int pos1 = rand() % freePositions.size();
        int pos2 = rand() % freePositions.size();
        
        if (pos1 != pos2) {
            int x1 = freePositions[pos1].first;
            int y1 = freePositions[pos1].second;
            int x2 = freePositions[pos2].first;
            int y2 = freePositions[pos2].second;
            
            if (abs(x1 - x2) >= 7 || abs(y1 - y2) >= 7) {
                gregorakis = new Hero(x1, y1, 'G', "Gregorakis", maze->getWidth(), maze->getHeight());
                asimenia = new Hero(x2, y2, 'S', "Asimenia", maze->getWidth(), maze->getHeight());
                
                // Remove used positions
                freePositions.erase(freePositions.begin() + max(pos1, pos2));
                freePositions.erase(freePositions.begin() + min(pos1, pos2));
                validPlacement = true;
            }
        }
        attempts++;
    }
    
    if (!validPlacement) {
        throw runtime_error("Could not place heroes with required distance");
    }
    
    // Place remaining objects
    if (freePositions.size() >= 3) {
        // Place key
        int keyPos = rand() % freePositions.size();
        key = new GameObject(freePositions[keyPos].first, freePositions[keyPos].second, 'K', ObjectType::KEY);
        freePositions.erase(freePositions.begin() + keyPos);
        
        // Place traps
        int trap1Pos = rand() % freePositions.size();
        trap1 = new GameObject(freePositions[trap1Pos].first, freePositions[trap1Pos].second, 'T', ObjectType::TRAP);
        freePositions.erase(freePositions.begin() + trap1Pos);
        
        int trap2Pos = rand() % freePositions.size();
        trap2 = new GameObject(freePositions[trap2Pos].first, freePositions[trap2Pos].second, 'T', ObjectType::TRAP);
    }
}

void Game::updateDisplay() {
    clear();
    
    // Display maze
    maze->display();
    
    // Display objects
    if (key && key->isActive()) {
        attron(COLOR_PAIR(2));
        mvaddch(key->getY(), key->getX(), key->getSymbol());
        attroff(COLOR_PAIR(2));
    }
    
    // Display the traps for Users
    if (trap1 && trap1->isActive() && trap1->getType() == ObjectType::TRAP) {
        attron(COLOR_PAIR(4));
        mvaddch(trap1->getY(), trap1->getX(), 'T');
        attroff(COLOR_PAIR(4));
    }

    if (trap2 && trap2->isActive() && trap2->getType() == ObjectType::TRAP) {
        attron(COLOR_PAIR(4)); 
        mvaddch(trap2->getY(), trap2->getX(), 'T');
        attroff(COLOR_PAIR(4));
    }
    
    // Display cages when activated
    if (cage1 && cage1->isVisible()) {
        attron(COLOR_PAIR(4));
        mvaddch(cage1->getY(), cage1->getX(), cage1->getSymbol());
        attroff(COLOR_PAIR(4));
    }
    
    if (cage2 && cage2->isVisible()) {
        attron(COLOR_PAIR(4));
        mvaddch(cage2->getY(), cage2->getX(), cage2->getSymbol());
        attroff(COLOR_PAIR(4));
    }
    
    // Display heroes
    if (gregorakis && !gregorakis->getIsTrapped()) {
        attron(COLOR_PAIR(1));
        mvaddch(gregorakis->getY(), gregorakis->getX(), gregorakis->getSymbol());
        attroff(COLOR_PAIR(1));
    }
    
    if (asimenia && !asimenia->getIsTrapped()) {
        attron(COLOR_PAIR(1));
        mvaddch(asimenia->getY(), asimenia->getX(), asimenia->getSymbol());
        attroff(COLOR_PAIR(1));
    }

    attron(COLOR_PAIR(5));
    if (wallsDisappearing) {
        mvprintw(maze->getHeight() + 4, 0, "Heroes found! Walls disappearing...");
    } else if (movingToLadder) {
        mvprintw(maze->getHeight() + 4, 0, "Moving to the ladder...");
    }
    attroff(COLOR_PAIR(5));
    
    refresh();
}

bool Game::isCagePosition(int x, int y) const {
    if (cage1 && cage1->isVisible() && cage1->isActive() && 
        cage1->getType() == ObjectType::CAGE &&
        cage1->getX() == x && cage1->getY() == y) {
        return true;
    }
    
    if (cage2 && cage2->isVisible() && cage2->isActive() && 
        cage2->getType() == ObjectType::CAGE &&
        cage2->getX() == x && cage2->getY() == y) {
        return true;
    }
    
    return false;
}

void Game::processHeroTurn(Hero* hero) {
    if (hero->getIsTrapped()) { // Trapped heroes can't move
        return; 
    }
    
    // Update hero's vision
    hero->updateVision(maze);
    
    // Get next move
    int keyX = -1, keyY = -1;
    if (key && key->isActive() && hero->canSeePosition(key->getX(), key->getY())) {
        keyX = key->getX();
        keyY = key->getY();
    }
    
    vector<pair<int, int>> visibleCages;
    if (cage1 && cage1->isVisible() && cage1->isActive() && 
        hero->canSeePosition(cage1->getX(), cage1->getY())) {
        visibleCages.push_back({cage1->getX(), cage1->getY()});
    }
    if (cage2 && cage2->isVisible() && cage2->isActive() && 
        hero->canSeePosition(cage2->getX(), cage2->getY())) {
        visibleCages.push_back({cage2->getX(), cage2->getY()});
    }
    
    pair<int, int> nextMove = hero->decideNextMove(maze, keyX, keyY, visibleCages);
    
    // Validate move
    bool canMove = true;
    
    // Check if it's a valid maze position
    if (maze->isWall(nextMove.first, nextMove.second)) {
        canMove = false;
    }
    
    if (canMove && isCagePosition(nextMove.first, nextMove.second)) {
        if (!hero->getHasKey()) {
            canMove = false;  // Cannot enter cage without key
        }
    }
    
    // Execute move if valid
    if (canMove) {
        hero->setPosition(nextMove.first, nextMove.second);
        checkCollisions(hero);
    }
    else {
        hero->notifyBlockedMove(nextMove.first, nextMove.second);
    }
}

void Game::checkCollisions(Hero* hero) {
    int heroX = hero->getX();
    int heroY = hero->getY();
    
    // Check key collision
    if (key && key->isActive() && key->getX() == heroX && key->getY() == heroY) {
        hero->setHasKey(true);
        key->setActive(false);
    }
    
    // Check trap collisions, traps are not visible to heroes
    if (trap1 && trap1->getType() == ObjectType::TRAP && trap1->isActive() &&
        trap1->getX() == heroX && trap1->getY() == heroY) {
        trap1->triggerTrap(); // Trap to Cage
        hero->setTrapped(true);
        cage1 = trap1;
    }
    
    if (trap2 && trap2->getType() == ObjectType::TRAP && trap2->isActive() &&
        trap2->getX() == heroX && trap2->getY() == heroY) {
        trap2->triggerTrap();
        hero->setTrapped(true);
        cage2 = trap2;
    }
    // Key opens the cage only from the same position 
    if (hero->getHasKey() && !hero->getIsTrapped()) {
        
        Hero* otherHero = (hero == gregorakis) ? asimenia : gregorakis;
        
        if (otherHero->getIsTrapped()) {
            bool rescued = false;
            
            if (cage1 && cage1->isVisible() && cage1->isActive() &&
                heroX == cage1->getX() && heroY == cage1->getY() &&
                otherHero->getX() == cage1->getX() && otherHero->getY() == cage1->getY()) {
                otherHero->setTrapped(false);
                cage1->setActive(false);
                cage1->setVisible(false);
                rescued = true;
            }
            
            if (!rescued && cage2 && cage2->isVisible() && cage2->isActive() &&
                heroX == cage2->getX() && heroY == cage2->getY() &&
                otherHero->getX() == cage2->getX() && otherHero->getY() == cage2->getY()) {
                otherHero->setTrapped(false);
                cage2->setActive(false);
                cage2->setVisible(false);
                rescued = true;
            }
            
            if (rescued) {
                hero->setHasKey(false); // Key consumed
				otherHero->setPosition(heroX, heroY);
                if (!heroesFound && 
                    gregorakis->getX() == asimenia->getX() && 
                    gregorakis->getY() == asimenia->getY() &&
                    !gregorakis->getIsTrapped() && !asimenia->getIsTrapped()) {
                    
                    heroesFound = true;
                    startWallDisappearing();
                }
            }
        }
    }
}

void Game::checkGameConditions() {
    if (heroesFound && !wallsDisappearing && 
        gregorakis->getX() == ladder->getX() && gregorakis->getY() == ladder->getY() &&
        asimenia->getX() == ladder->getX() && asimenia->getY() == ladder->getY()) {
        gameWon = true;
        return;
    }
	
    // Check if heroes are together 
    if (!heroesFound && 
        gregorakis->getX() == asimenia->getX() && 
        gregorakis->getY() == asimenia->getY() &&
        !gregorakis->getIsTrapped() && !asimenia->getIsTrapped()) {
        heroesFound = true;
        startWallDisappearing();
        return;
    }
    
    // Check if game is lost
    if (turns >= 1000) {
        gameLost = true;
        return;
    }
    
    // Check if both heroes are trapped with no way to escape
    if (gregorakis->getIsTrapped() && asimenia->getIsTrapped()) {
        gameLost = true;
        return;
    }
    
    // Check if one hero is trapped and the other doesn't have the key
    // and the key is not accessible (losing condition)
    if ((gregorakis->getIsTrapped() && !asimenia->getHasKey()) ||
        (asimenia->getIsTrapped() && !gregorakis->getHasKey())) {
        // Additional check if the key is still available
        if (!key || !key->isActive()) {
            gameLost = true;
            return;
        }
    }
}

void Game::startWallDisappearing() {
    wallsDisappearing = true;
    wallDisappearCounter = 0;
    wallsToRemove.clear();
    
    for (int y = 1; y < maze->getHeight() - 1; y++) {
        for (int x = 1; x < maze->getWidth() - 1; x++) {
            if (maze->isWall(x, y)) {
                wallsToRemove.push_back({x, y});
            }
        }
    }  
    cout << "Heroes found! Walls disappearing... Total internal walls: " 
              << wallsToRemove.size() << endl;
}

void Game::updateWallDisappearing() {
    if (!wallsDisappearing) return;
    if (wallDisappearCounter < wallsToRemove.size()) {
        int wallIndex = wallDisappearCounter;
        int x = wallsToRemove[wallIndex].first;
        int y = wallsToRemove[wallIndex].second;
        
        maze->removeWall(x, y);
        wallDisappearCounter++;
        
        cout << "Wall disappeared at (" << x << "," << y << ") - " 
                  << wallDisappearCounter << "/" << wallsToRemove.size() << endl;
    } else {
        // The inside walls disappeared
        wallsDisappearing = false;
        startMovingToLadder();
    }
}

void Game::startMovingToLadder() {
    movingToLadder = true;
    cout << "Heroes now moving to ladder using shortest path..." << endl;
}

void Game::moveHeroesToLadder() {
    if (!movingToLadder) return;
    
    int ladderX = ladder->getX();
    int ladderY = ladder->getY();
    
    int gX = gregorakis->getX();
    int gY = gregorakis->getY();
    
    if (gX != ladderX) {
        gX += (ladderX > gX) ? 1 : -1;
    } else if (gY != ladderY) {
        gY += (ladderY > gY) ? 1 : -1;
    }
    
    gregorakis->setPosition(gX, gY);
    
    int aX = asimenia->getX();
    int aY = asimenia->getY();
    
    if (aX != ladderX) {
        aX += (ladderX > aX) ? 1 : -1;
    } else if (aY != ladderY) {
        aY += (ladderY > aY) ? 1 : -1;
    }
    
    asimenia->setPosition(aX, aY);
    
    // Check if they are both reached the ladder
    if (gregorakis->getX() == ladderX && gregorakis->getY() == ladderY &&
        asimenia->getX() == ladderX && asimenia->getY() == ladderY) {
        gameWon = true;
    }
}

bool Game::isValidPosition(int x, int y) {
    return maze->isValidPosition(x, y) && !maze->isWall(x, y);
}

bool Game::isPositionOccupied(int x, int y) {
    if (gregorakis && gregorakis->getX() == x && gregorakis->getY() == y) return true;
    if (asimenia && asimenia->getX() == x && asimenia->getY() == y) return true;
    if (key && key->isActive() && key->getX() == x && key->getY() == y) return true;
    if (trap1 && trap1->isActive() && trap1->getX() == x && trap1->getY() == y) return true;
    if (trap2 && trap2->isActive() && trap2->getX() == x && trap2->getY() == y) return true;
    if (ladder && ladder->getX() == x && ladder->getY() == y) return true;
    return false;
}

int Game::manhattanDistance(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

void Game::run() {
    while (!isGameOver()) {
        updateDisplay();
        
        // Process game phases
        if (wallsDisappearing) {
            updateWallDisappearing();
        } else if (movingToLadder) {
            moveHeroesToLadder();
        } else {
            // Normal gameplay
            processHeroTurn(gregorakis);
            processHeroTurn(asimenia);
        }
        
        // Check game conditions
        checkGameConditions();
        
        turns++;
        
        // Timer for walls and players
        if (wallsDisappearing || movingToLadder) {
            usleep(50000); // 50ms
        } else {
            usleep(130000); // 130ms
        }
        
        // Check for user input to quit
        int ch = getch();
        if (ch == 'q' || ch == 'Q') {
            break;
        }
    }
    
    // Display final result
    clear();
    if (gameWon) {
        attron(COLOR_PAIR(3));
        mvprintw(maze->getHeight()/2, maze->getWidth()/2 - 15, "Congratulations! The heroes saved the kingdom!");
        attroff(COLOR_PAIR(3));
    } else {
        attron(COLOR_PAIR(1));
        mvprintw(maze->getHeight()/2, maze->getWidth()/2 - 12, "Game Over! The kingdom has fallen...");
        attroff(COLOR_PAIR(1));
    }
    mvprintw(maze->getHeight()/2 + 2, maze->getWidth()/2 - 12, "Press any key to exit...");
    refresh();
    
    nodelay(stdscr, FALSE);
    getch();
}

bool Game::isGameOver() const {
    return gameWon || gameLost;
}

bool Game::isGameWon() const {
    return gameWon;
}