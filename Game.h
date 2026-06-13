#pragma once
#include <map>

#include "CommandParser.h"
#include "Enemy.h"
#include "Player.h"
#include "World.h"

class Game {
 public:
  World world;
  Player player;
  bool isRunning;
  bool selfDestructActive = false;
  std::map<std::string, Item> itemTemplates;
  std::map<std::string, Enemy> enemyTemplates;
  std::string previousRoomId;
  int selfDestructTimer = 0;
  void dropItem(const std::string& itemName);

  Game();
  void init();
  void run();
  void processCommand(const Command& cmd);
  void combat(Enemy& enemy);
  void showHelp();
  static void showMainMenu();
  static void runGame();
  void waitForEnter();
};
