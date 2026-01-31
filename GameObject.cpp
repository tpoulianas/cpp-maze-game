#include "GameObject.h"

GameObject::GameObject(int posX, int posY, char sym, ObjectType objType) 
    : x(posX), y(posY), symbol(sym), type(objType), visible(true), active(true) {
    
    // Traps start invisible to heroes
    if (type == ObjectType::TRAP) {
        visible = false;
    }
}

GameObject::~GameObject() {
}

void GameObject::setPosition(int newX, int newY) {
    x = newX;
    y = newY;
}

void GameObject::triggerTrap() {
    if (type == ObjectType::TRAP && active) {
        symbol = 'C'; // Change to cage symbol
        type = ObjectType::CAGE;
        visible = true; // Cage becomes visible
    }
}