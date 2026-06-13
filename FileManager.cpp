#include "FileManager.h"

#include <windows.h>

#include <fstream>
#include <iostream>

#include "../lib/json.hpp"

using json = nlohmann::json;

static std::string utf8ToAnsi(const std::string& utf8str) {
  if (utf8str.empty()) return {};
  int wideLen =
      MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), -1, nullptr, 0);
  if (wideLen == 0) return utf8str;
  std::wstring wideStr(wideLen, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), -1, &wideStr[0], wideLen);
  int ansiLen = WideCharToMultiByte(1251, 0, wideStr.c_str(), -1, nullptr, 0,
                                    nullptr, nullptr);
  if (ansiLen == 0) return utf8str;
  std::string ansiStr(ansiLen, '\0');
  WideCharToMultiByte(1251, 0, wideStr.c_str(), -1, &ansiStr[0], ansiLen,
                      nullptr, nullptr);
  ansiStr.resize(ansiLen - 1);
  return ansiStr;
}

std::map<std::string, Item> FileManager::loadItems(const std::string& path) {
  std::map<std::string, Item> items;
  std::ifstream file(path);
  if (!file) {
    std::cout << "Cannot open items file: " << path << "\n";
    return items;
  }
  try {
    json j;
    file >> j;
    for (auto& [name, data] : j.items()) {
      std::string key = utf8ToAnsi(name);
      std::string typeStr = utf8ToAnsi(data.value("type", "quest"));
      ItemType type = ItemType::QUEST;
      if (typeStr == "consumable")
        type = ItemType::CONSUMABLE;
      else if (typeStr == "weapon")
        type = ItemType::WEAPON;
      else if (typeStr == "key")
        type = ItemType::KEY;
      else if (typeStr == "armor")
        type = ItemType::ARMOR;
      else if (typeStr == "ammo")
        type = ItemType::AMMO;
      else if (typeStr == "currency")
        type = ItemType::CURRENCY;
      else if (typeStr == "terminal")
        type = ItemType::TERMINAL;
      int value = data.value("value", 0);
      std::string desc = utf8ToAnsi(data.value("description", ""));
      std::string caliber = utf8ToAnsi(data.value("caliber", ""));
      int ammoPerShot = data.value("ammo_per_shot", 0);
      items[key] = Item(key, desc, type, value, caliber, ammoPerShot);
    }
  } catch (json::parse_error& e) {
    std::cout << "Parse error in " << path << ": " << e.what() << "\n";
  }
  return items;
}

std::map<std::string, Enemy> FileManager::loadEnemies(
    const std::string& path, const std::map<std::string, Item>& itemTemplates) {
  std::map<std::string, Enemy> enemies;
  std::ifstream file(path);
  if (!file) {
    std::cout << "Cannot open enemies file: " << path << "\n";
    return enemies;
  }
  try {
    json j;
    file >> j;
    for (auto& [id, e] : j.items()) {
      std::string key = utf8ToAnsi(id);
      std::string name = utf8ToAnsi(e["name"].get<std::string>());
      int health = e["health"];
      int attackPower = e["attackPower"];
      int escapeChance = e["escapeChance"];
      std::vector<Item> loot;
      for (auto& lootName : e["loot"]) {
        std::string ln = utf8ToAnsi(lootName.get<std::string>());
        if (itemTemplates.count(ln)) loot.push_back(itemTemplates.at(ln));
      }
      enemies[key] = Enemy(name, health, attackPower, escapeChance, loot);
    }
  } catch (json::parse_error& e) {
    std::cout << "Parse error in " << path << ": " << e.what() << "\n";
  }
  return enemies;
}

void FileManager::loadWorld(
    const std::string& path, World& world,
    const std::map<std::string, Item>& itemTemplates,
    const std::map<std::string, Enemy>& enemyTemplates) {
  std::ifstream file(path);
  if (!file) {
    std::cout << "Cannot open world file: " << path << "\n";
    return;
  }
  try {
    json j;
    file >> j;
    world.startRoomId = utf8ToAnsi(j.value("start_room", "entrance"));
    world.winRoomId = utf8ToAnsi(j["win_condition"].value("room", "surface"));
    world.winRequiredItem =
        utf8ToAnsi(j["win_condition"].value("has_item", "антидот"));

    for (auto& [id, r] : j["rooms"].items()) {
      std::string roomId = utf8ToAnsi(id);
      std::string roomName = utf8ToAnsi(r["name"].get<std::string>());
      std::string roomDesc = utf8ToAnsi(r["description"].get<std::string>());
      Room room(roomId, roomName, roomDesc);
      for (auto& [dir, target] : r["exits"].items())
        room.exits[utf8ToAnsi(dir)] = utf8ToAnsi(target);
      for (auto& itemName : r["items"]) {
        std::string name = utf8ToAnsi(itemName.get<std::string>());
        if (itemTemplates.count(name)) room.addItem(itemTemplates.at(name));
      }
      for (auto& enemyName : r["enemies"]) {
        std::string name = utf8ToAnsi(enemyName.get<std::string>());
        if (enemyTemplates.count(name)) room.addEnemy(enemyTemplates.at(name));
      }
      if (r.contains("requires_key"))
        room.requiredKey = utf8ToAnsi(r["requires_key"].get<std::string>());
      if (r.contains("requires_code"))
        room.requiredCode = utf8ToAnsi(r["requires_code"].get<std::string>());
      if (r.contains("terminal")) {
        room.hasTerminal = true;
        room.terminalId = utf8ToAnsi(r["terminal"].get<std::string>());
      }
      world.addRoom(room);
    }
  } catch (json::parse_error& e) {
    std::cout << "Parse error in " << path << ": " << e.what() << "\n";
  }
}
