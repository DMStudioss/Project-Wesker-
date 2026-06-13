#pragma once
#include <map>
#include <string>
#include <vector>

#include "Enemy.h"
#include "Item.h"

class Room {
 public:
  std::string id;
  std::string name;
  std::string description;
  std::map<std::string, std::string> exits;
  std::vector<Item> items;
  std::vector<Enemy> enemies;
  std::string requiredKey;
  std::string requiredCode;
  bool hasTerminal = false;
  std::string terminalId;

  Room();
  Room(const std::string& id, const std::string& name, const std::string& desc);
  void addItem(const Item& item);
  void removeItem(const std::string& itemName);
  void addEnemy(const Enemy& enemy);
  void removeDeadEnemies();
};
