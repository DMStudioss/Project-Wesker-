#include "World.h"

#include <iostream>

#include "Player.h"

World::World() {}

void World::addRoom(const Room& room) { rooms[room.id] = room; }

Room* World::getCurrentRoom() {
  auto it = rooms.find(currentRoomId);
  return it != rooms.end() ? &it->second : nullptr;
}

bool World::movePlayer(const std::string& direction, const Player& player,
                       std::string& message) {
  Room* room = getCurrentRoom();
  if (!room) return false;
  auto it = room->exits.find(direction);
  if (it == room->exits.end()) {
    message = "Туда нельзя идти.";
    return false;
  }
  std::string targetId = it->second;
  if (rooms.find(targetId) == rooms.end()) {
    message = "Ошибка мира.";
    return false;
  }
  Room& targetRoom = rooms[targetId];
  if (!targetRoom.requiredKey.empty()) {
    if (!player.hasItem(targetRoom.requiredKey)) {
      message = "Дверь заперта. Требуется: " + targetRoom.requiredKey;
      return false;
    }
  }
  if (!targetRoom.requiredCode.empty()) {
    std::cout << "Введите код: ";
    std::string input;
    std::getline(std::cin, input);
    if (input != targetRoom.requiredCode) {
      message = "Неверный код.";
      return false;
    }
  }
  currentRoomId = targetId;
  return true;
}
