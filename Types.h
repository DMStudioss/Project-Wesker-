#pragma once
#include <string>
#include <vector>

enum class ItemType {
  WEAPON,
  CONSUMABLE,
  KEY,
  QUEST,
  ARMOR,
  AMMO,
  CURRENCY,
  TERMINAL
};

struct Command {
  enum Type {
    MOVE,
    TAKE,
    USE,
    ATTACK,
    LOOK,
    INVENTORY,
    QUIT,
    HELP,
    DROP,
    UNKNOWN
  };
  Type type;
  std::vector<std::string> args;
};
