#pragma once
#include <iostream>
#include <string>
#include <vector>

#include "Item.h"

class Player {
 public:
  std::string name;
  int health;
  int maxHealth;
  int attackPower;
  std::vector<Item> inventory;
  std::string equippedWeaponName;
  bool armorActive;
  int ammo9mm;
  int ammo12g;
  int poisonTurns = 0;

  Player();
  bool pickUpItem(const Item& item);
  bool useItem(const std::string& itemName);
  void equipWeapon(const std::string& weaponName);
  int getWeaponDamage() const;
  bool hasAmmoForWeapon() const;
  void consumeAmmo();
  bool hasItem(const std::string& name) const;
  void removeItem(const std::string& name);
  Item* getEquippedWeapon();
};
