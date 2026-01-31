#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

enum class ObjectType {
    TRAP,
    CAGE, 
    KEY,
    LADDER
};

class GameObject {
private:
    int x, y;
    char symbol;
    ObjectType type;
    bool visible;
    bool active;
    
public:
    GameObject(int posX, int posY, char sym, ObjectType objType);
    ~GameObject();
    
    // Position
    int getX() const { return x; }
    int getY() const { return y; }
    void setPosition(int newX, int newY);
    
    // Appearance
    char getSymbol() const { return symbol; }
    void setSymbol(char newSymbol) { symbol = newSymbol; }
    
    // State
    ObjectType getType() const { return type; }
    bool isVisible() const { return visible; }
    bool isActive() const { return active; }
    
    void setVisible(bool vis) { visible = vis; }
    void setActive(bool act) { active = act; }
    
    // Convert trap to cage
    void triggerTrap(); 
};

#endif 