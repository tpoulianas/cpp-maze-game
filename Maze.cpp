#include "Maze.h"
#include <fstream>
#include <iostream>
#include <ncurses.h>

using namespace std;

Maze::Maze(const string& filename) : width(0), height(0), ladderX(-1), ladderY(-1) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Cannot open maze file: " + filename);
    }
    
    string line;
    vector<string> tempGrid;
    
    // Read all lines
    while (getline(file, line)) {
        tempGrid.push_back(line);
        if (line.length() > width) {
            width = line.length();
        }
    }
    file.close();
    
    height = tempGrid.size();
    
    // Initialize grid and find ladder
    grid.resize(height);
    for (int i = 0; i < height; i++) {
        grid[i].resize(width, '*'); 
        for (int j = 0; j < tempGrid[i].length(); j++) {
            grid[i][j] = tempGrid[i][j];
            if (tempGrid[i][j] == 'L') {
                ladderX = j;
                ladderY = i;
                grid[i][j] = ' '; // Convert L to space for movement
            }
        }
    }
    
    if (ladderX == -1 || ladderY == -1) {
        throw runtime_error("No ladder found in maze file");
    }
}

Maze::~Maze() {
    // Vectors automatically clean up
}

char Maze::getCell(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return '*'; // Out of bounds is wall
    }
    return grid[y][x];
}

void Maze::setCell(int x, int y, char value) {
    if (isValidPosition(x, y)) {
        grid[y][x] = value;
    }
}

bool Maze::isWall(int x, int y) const {
    return getCell(x, y) == '*';
}

bool Maze::isValidPosition(int x, int y) const {
    return x >= 0 && x < width && y >= 0 && y < height;
}

void Maze::removeWall(int x, int y) {
    if (isValidPosition(x, y) && isWall(x, y)) {
        setCell(x, y, ' ');
    }
}

vector<pair<int, int>> Maze::getAllWalls() const {
    vector<pair<int, int>> walls;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (isWall(x, y)) {
                walls.push_back({x, y});
            }
        }
    }
    return walls;
}

void Maze::display() const {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            char cell = grid[y][x];
            if (x == ladderX && y == ladderY) {
                attron(COLOR_PAIR(3)); // Special color for ladder
                mvaddch(y, x, 'L');
                attroff(COLOR_PAIR(3));
            } else {
                mvaddch(y, x, cell);
            }
        }
    }
}