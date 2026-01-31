#ifndef MAZE_H
#define MAZE_H

#include <vector>
#include <string>

class Maze {
private:
    std::vector<std::vector<char>> grid;
    int width;
    int height;
    int ladderX, ladderY;
    
public:
    Maze(const std::string& filename);
    ~Maze();
    
    char getCell(int x, int y) const;
    void setCell(int x, int y, char value);
    bool isWall(int x, int y) const;
    bool isValidPosition(int x, int y) const;
    
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getLadderX() const { return ladderX; }
    int getLadderY() const { return ladderY; }
    
    void removeWall(int x, int y);
    std::vector<std::pair<int, int>> getAllWalls() const;
    
    void display() const;
};

#endif 