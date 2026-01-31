#include <iostream>
#include <cstdlib>
#include <ctime>
#include "Game.h"

using namespace std;

int main(int argc, char* argv[]) {
    // Check command line arguments
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <maze_file>" << endl;
        cerr << "Example: " << argv[0] << " map1.txt" << endl;
        return 1;
    }
    
    // Initialize random seed
    srand(time(nullptr));
    
    try {
        // Create and run the game
        Game game(argv[1]);
        
        cout << "Starting 'Gregorakis and Asimenia: A Love Story'" << endl;
        cout << "Press 'q' to quit during gameplay" << endl;
        cout << "Press any key to start..." << endl;
        cin.get();
        
        game.run();
        
        if (game.isGameWon()) {
            cout << "\nCongratulations! The heroes saved the kingdom!" << endl;
        } else {
            cout << "\nGame Over! The kingdom has fallen..." << endl;
        }
        
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}