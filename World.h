#pragma once
#include <map>
#include <string>

#include "Room.h"

class World {
 public:
  std::map<std::string, Room> rooms;
  std::string startRoomId;
  std::string winRoomId;
  std::string winRequiredItem;
  std::string currentRoomId;

  World();
  void addRoom(const Room& room);
  Room* getCurrentRoom();
  bool movePlayer(const std::string& direction, const Player& player,
                  std::string& message);
};
