#pragma once
#include <map>
#include <string>

#include "Enemy.h"
#include "Item.h"
#include "Player.h"
#include "World.h"

class FileManager {
 public:
  static std::map<std::string, Item> loadItems(const std::string& path);
  static std::map<std::string, Enemy> loadEnemies(
      const std::string& path,
      const std::map<std::string, Item>& itemTemplates);
  static void loadWorld(const std::string& path, World& world,
                        const std::map<std::string, Item>& itemTemplates,
                        const std::map<std::string, Enemy>& enemyTemplates);
};
