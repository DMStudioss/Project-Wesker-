#include "Player.h"

#include <algorithm>

#include "Enemy.h"

Player::Player() {
  name = "Оперативник";
  maxHealth = 100;
  health = maxHealth;
  attackPower = 5;
  armorActive = false;
  ammo9mm = 30;
  ammo12g = 0;
}

bool Player::pickUpItem(const Item& item) {
  if (item.type == ItemType::AMMO) {
    if (item.caliber == "9mm") {
      ammo9mm += item.value;
      std::cout << "Подобрано " << item.value
                << " патронов 9мм. Всего: " << ammo9mm << "\n";
    } else if (item.caliber == "12g") {
      ammo12g += item.value;
      std::cout << "Подобрано " << item.value
                << " патронов 12 калибра. Всего: " << ammo12g << "\n";
    }
    return true;
  }
  if (inventory.size() >= 8) {
    std::cout << "Инвентарь полон!\n";
    return false;
  }
  inventory.push_back(item);
  std::cout << "Вы подобрали: " << item.name << "\n";
  return true;
}

bool Player::useItem(const std::string& itemName) {
  for (auto it = inventory.begin(); it != inventory.end(); ++it) {
    if (it->name == itemName) {
      if (it->type == ItemType::CONSUMABLE || it->type == ItemType::ARMOR) {
        if (it->use(*this)) {
          inventory.erase(it);
          return true;
        }
      } else if (it->type == ItemType::QUEST) {
        it->use(*this);
        return false;
      } else if (it->type == ItemType::WEAPON) {
        equipWeapon(itemName);
        return false;
      }
      return false;
    }
  }
  std::cout << "Предмет не найден.\n";
  return false;
}

void Player::equipWeapon(const std::string& weaponName) {
  for (auto& item : inventory) {
    if (item.name == weaponName && item.type == ItemType::WEAPON) {
      equippedWeaponName = item.name;
      std::cout << "Экипировано: " << item.name << " (урон " << item.value
                << ")\n";
      return;
    }
  }
  std::cout << "Оружие не найдено.\n";
}

Item* Player::getEquippedWeapon() {
  if (equippedWeaponName.empty()) return nullptr;
  for (auto& item : inventory) {
    if (item.name == equippedWeaponName) return &item;
  }
  return nullptr;
}

int Player::getWeaponDamage() const {
  if (equippedWeaponName.empty()) return attackPower;
  for (auto& item : inventory) {
    if (item.name == equippedWeaponName) return item.value;
  }
  return attackPower;
}

bool Player::hasAmmoForWeapon() const {
  if (equippedWeaponName.empty()) return true;
  for (auto& item : inventory) {
    if (item.name == equippedWeaponName) {
      if (item.caliber == "9mm") return ammo9mm >= item.ammoPerShot;
      if (item.caliber == "12g") return ammo12g >= item.ammoPerShot;
      return true;
    }
  }
  return true;
}

void Player::consumeAmmo() {
  if (equippedWeaponName.empty()) return;
  for (auto& item : inventory) {
    if (item.name == equippedWeaponName) {
      if (item.caliber == "9mm")
        ammo9mm -= item.ammoPerShot;
      else if (item.caliber == "12g")
        ammo12g -= item.ammoPerShot;
      break;
    }
  }
}

bool Player::hasItem(const std::string& name) const {
  return std::any_of(inventory.begin(), inventory.end(),
                     [&](const Item& i) { return i.name == name; });
}

void Player::removeItem(const std::string& name) {
  inventory.erase(std::remove_if(inventory.begin(), inventory.end(),
                                 [&](const Item& i) { return i.name == name; }),
                  inventory.end());
}
