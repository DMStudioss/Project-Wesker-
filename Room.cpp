#include "Room.h"

#include <algorithm>

Room::Room() {}
Room::Room(const std::string& id, const std::string& name,
           const std::string& desc)
    : id(id), name(name), description(desc) {}

void Room::addItem(const Item& item) { items.push_back(item); }
void Room::removeItem(const std::string& itemName) {
  items.erase(std::remove_if(items.begin(), items.end(),
                             [&](const Item& i) { return i.name == itemName; }),
              items.end());
}
void Room::addEnemy(const Enemy& enemy) { enemies.push_back(enemy); }
void Room::removeDeadEnemies() {
  enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
                               [](const Enemy& e) { return !e.isAlive(); }),
                enemies.end());
}
