#pragma once
#include <string>
#include <vector>

#include "Item.h"

class Enemy {
 public:
  std::string name;
  int health;
  int maxHealth;
  int attackPower;
  int escapeChance;
  std::vector<Item> loot;

  Enemy();
  Enemy(const std::string& name, int hp, int atk, int escape,
        const std::vector<Item>& loot);
  void takeDamage(int amount);
  bool isAlive() const;
  void attack(Player& player);
};
