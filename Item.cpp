#include "Item.h"

#include <algorithm>
#include <iostream>

#include "Player.h"

Item::Item() : type(ItemType::QUEST), value(0), ammoPerShot(0) {}

Item::Item(const std::string& name, const std::string& desc, ItemType type,
           int value, const std::string& caliber, int ammoPerShot)
    : name(name),
      description(desc),
      type(type),
      value(value),
      caliber(caliber),
      ammoPerShot(ammoPerShot) {}

bool Item::use(Player& player) {
  if (type == ItemType::CONSUMABLE) {
    player.health = std::min(player.maxHealth, player.health + value);
    return true;
  }
  if (type == ItemType::ARMOR) {
    player.armorActive = true;
    std::cout << "Вы надели броню. Урон снижен на 25%.\n";
    return true;
  }
  if (type == ItemType::QUEST) {
    std::cout << description << "\n";
    return false;
  }
  return false;
}
