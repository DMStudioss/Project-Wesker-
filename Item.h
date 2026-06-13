#pragma once
#include <string>

#include "Types.h"

class Item {
 public:
  std::string name;
  std::string description;
  ItemType type;
  int value;
  std::string caliber;
  int ammoPerShot;

  Item();
  Item(const std::string& name, const std::string& desc, ItemType type,
       int value, const std::string& caliber = "", int ammoPerShot = 0);
  bool use(class Player& player);
};
