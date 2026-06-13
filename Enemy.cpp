#include "Enemy.h"

#include <windows.h>

#include <algorithm>
#include <iostream>

#include "Player.h"
#undef max
#undef min

Enemy::Enemy() : health(0), maxHealth(0), attackPower(0), escapeChance(0) {}

Enemy::Enemy(const std::string& name, int hp, int atk, int escape,
             const std::vector<Item>& loot)
    : name(name),
      health(hp),
      maxHealth(hp),
      attackPower(atk),
      escapeChance(escape),
      loot(loot) {}

void Enemy::takeDamage(int amount) {
  health = std::max(0, health - amount);
  std::cout << name << " получает " << amount << " урона. HP: " << health << "/"
            << maxHealth << "\n";
}

bool Enemy::isAlive() const { return health > 0; }

void Enemy::attack(Player& player) {
  int dmg = attackPower;
  if (player.armorActive) dmg = dmg * 3 / 4;

  if (name == "÷ербер") {
    int chance = rand() % 100;
    if (chance < 30) {
      std::cout << name << " атакует дважды!\n";
      player.health = std::max(0, player.health - dmg);
      if (player.health > 0) {
        player.health = std::max(0, player.health - dmg);
      }
    } else {
      std::cout << name << " атакует вас на " << dmg << " урона.\n";
      player.health = std::max(0, player.health - dmg);
    }
  } else if (name == "ћутант") {
    std::cout << name << " атакует вас на " << dmg << " урона.\n";
    player.health = std::max(0, player.health - dmg);
    int poisonChance = rand() % 100;
    if (poisonChance < 40) {
      player.poisonTurns = 2;
      std::cout << "¬ас отравили! -5 HP в начале следующих 2 ходов.\n";
    }
  } else if (name == "ѕрародитель") {
    std::cout << name << " атакует вас на " << dmg << " урона.\n";
    player.health = std::max(0, player.health - dmg);
    if (health < maxHealth && health < 50) {
      health = std::min(maxHealth, health + 5);
      std::cout << name << " восстанавливает 5 HP. HP: " << health << "/"
                << maxHealth << "\n";
    }
  } else {
    std::cout << name << " атакует вас на " << dmg << " урона.\n";
    player.health = std::max(0, player.health - dmg);
  }
}
