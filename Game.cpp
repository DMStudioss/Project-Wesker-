#include "Game.h"

#include <windows.h>

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <limits>
#include <sstream>

#include "FileManager.h"
#undef max
#undef min

Game::Game() : isRunning(false) {}

void Game::waitForEnter() {
  std::cout << "\033[1;30m[Нажмите Enter...]\033[0m";
  std::cin.ignore(10000, '\n');
  std::cin.get();
}

void Game::init() {
  srand(static_cast<unsigned>(time(nullptr)));
  itemTemplates = FileManager::loadItems("data/items.json");
  enemyTemplates = FileManager::loadEnemies("data/enemies.json", itemTemplates);
  FileManager::loadWorld("data/rooms.json", world, itemTemplates,
                         enemyTemplates);

  if (itemTemplates.empty() || enemyTemplates.empty() || world.rooms.empty()) {
    std::cout << "Ошибка загрузки данных. Проверьте файлы в папке data.\n";
    isRunning = false;
    return;
  }

  world.currentRoomId = world.startRoomId;
  previousRoomId = "";
  selfDestructActive = false;
  selfDestructTimer = 0;

  auto pistolIt = itemTemplates.find("пистолет");
  if (pistolIt != itemTemplates.end()) {
    player.inventory.push_back(pistolIt->second);
    player.equippedWeaponName = pistolIt->second.name;
  } else {
    std::cout << "Ошибка: предмет 'пистолет' не найден.\n";
    isRunning = false;
    return;
  }

  player.poisonTurns = 0;

  isRunning = true;
  system("cls");
  std::cout << "\033[1;36m";
  std::cout << "Вы оперативник спецподразделения получаете приказ проникнуть в подземный комплекс «Зонт», "
               "откуда перестаёт выходить на связь персонал. \n";
  std::cout << "Ваша задача найти антидот в реакторной \n";
  std::cout << "и уничтожить комплекс, чтобы вирус не распространился на "
               "поверхность.\n";
  std::cout << "Вертолёт высаживает вас у входа, вы сжимаете пистолет и "
               "входите внутрь.\033[0m\n\n";
  waitForEnter();
}

void Game::run() {
  while (isRunning && player.health > 0) {
    Room* room = world.getCurrentRoom();
    if (!room) break;

    std::string terminalId;
    if (room->hasTerminal) terminalId = room->terminalId;

    std::cout << "\n\033[1;36m=== " << room->name << " ===\033[0m\n"
              << room->description << "\n";

    if (selfDestructActive) {
      std::cout << "\033[1;31m[САМОУНИЧТОЖЕНИЕ] Осталось ходов: "
                << selfDestructTimer << "\033[0m\n";
    }

    if (!room->enemies.empty()) {
      std::cout << "\033[1;31mВраги:\033[0m\n";
      for (size_t i = 0; i < room->enemies.size(); ++i)
        std::cout << "  " << (i + 1) << ". " << room->enemies[i].name << " (HP "
                  << room->enemies[i].health << ")\n";
    }
    if (!room->items.empty()) {
      std::cout << "\033[1;33mПредметы:\033[0m ";
      for (auto& i : room->items) std::cout << i.name << " ";
      std::cout << "\n";
    }
    if (!terminalId.empty())
      std::cout << "\033[1;36mЗдесь установлен терминал.\033[0m\n";

    if (!room->enemies.empty()) {
      std::cout << "\033[1;31mВраги заметили вас! Можно сделать одно быстрое "
                   "действие "
                   "(взять/использовать/инвентарь/осмотреться/идти/"
                   "выбросить).\033[0m\n";
      std::cout << "\033[1;37m> \033[0m";
      std::string input;
      std::getline(std::cin, input);
      if (!input.empty()) {
        Command cmd = CommandParser::parse(input);
        if (cmd.type == Command::MOVE) {
          processCommand(cmd);
          if (selfDestructActive) {
            selfDestructTimer--;
            if (selfDestructTimer <= 0) {
              std::cout
                  << "\033[1;31mВремя вышло! Комплекс взорвался.\033[0m\n";
              player.health = 0;
              break;
            }
          }
          continue;
        } else if (cmd.type == Command::TAKE || cmd.type == Command::USE ||
                   cmd.type == Command::INVENTORY ||
                   cmd.type == Command::LOOK || cmd.type == Command::DROP) {
          processCommand(cmd);
        }
        if (selfDestructActive && cmd.type != Command::LOOK &&
            cmd.type != Command::HELP) {
          selfDestructTimer--;
          if (selfDestructTimer <= 0) {
            std::cout << "\033[1;31mВремя вышло! Комплекс взорвался.\033[0m\n";
            player.health = 0;
            break;
          }
        }
      }
      while (!room->enemies.empty() && player.health > 0) {
        Enemy& enemy = room->enemies.front();
        std::cout << "\n\033[1;31mВас атакует " << enemy.name << "!\033[0m\n";
        std::string preCombatRoom = world.currentRoomId;
        combat(enemy);
        if (selfDestructActive) {
          selfDestructTimer--;
          if (selfDestructTimer <= 0) {
            std::cout << "\033[1;31mВремя вышло! Комплекс взорвался.\033[0m\n";
            player.health = 0;
            break;
          }
        }
        if (player.health <= 0) break;
        if (world.currentRoomId != preCombatRoom) {
          break;
        }
        room->removeDeadEnemies();
        if (room->enemies.empty()) {
          std::cout << "\033[1;32mВсе враги в комнате повержены.\033[0m\n";
          waitForEnter();
        }
      }
      if (player.health <= 0) break;
    }

    std::cout << "\033[1;37m> \033[0m";
    std::string input;
    std::getline(std::cin, input);
    if (input.empty()) continue;

    Command cmd = CommandParser::parse(input);
    processCommand(cmd);

    if (world.currentRoomId == world.winRoomId &&
        player.hasItem(world.winRequiredItem)) {
      std::cout
          << "\033[1;32m\nВы победили! Антидот у вас. Вы выбрались!\033[0m\n";
      isRunning = false;
    }
    waitForEnter();
  }
  if (player.health <= 0) {
    std::cout << "\033[1;31m\nВы погибли... Игра окончена.\033[0m\n";
  }
}

void Game::processCommand(const Command& cmd) {
  Room* room = world.getCurrentRoom();
  if (!room) return;

  if (selfDestructActive && cmd.type != Command::LOOK &&
      cmd.type != Command::HELP) {
    selfDestructTimer--;
    if (selfDestructTimer <= 0) {
      std::cout << "\033[1;31mВремя вышло! Комплекс взорвался.\033[0m\n";
      player.health = 0;
      return;
    }
  }

  switch (cmd.type) {
    case Command::MOVE: {
      if (cmd.args.empty()) {
        std::cout << "Куда идти? (север, юг, запад, восток, вверх, вниз)\n";
        break;
      }
      std::string dir;
      std::vector<std::string> dirsToCheck = {cmd.args[0]};
      if (cmd.args.size() > 1) dirsToCheck.push_back(cmd.args[1]);

      for (auto& d : dirsToCheck) {
        if (d == "с" || d == "север") {
          dir = "north";
          break;
        } else if (d == "ю" || d == "юг") {
          dir = "south";
          break;
        } else if (d == "з" || d == "запад") {
          dir = "west";
          break;
        } else if (d == "в" || d == "восток") {
          dir = "east";
          break;
        } else if (d == "вверх" || d == "up") {
          dir = "up";
          break;
        } else if (d == "вниз" || d == "down") {
          dir = "down";
          break;
        } else if (d == "выход" || d == "exit") {
          dir = "exit";
          break;
        } else if (d == "north" || d == "south" || d == "west" || d == "east" ||
                   d == "up" || d == "down") {
          dir = d;
          break;
        }
      }

      if (dir.empty()) {
        std::cout << "Неизвестное направление.\n";
        break;
      }

      std::string oldRoomId = world.currentRoomId;
      std::string msg;
      if (world.movePlayer(dir, player, msg)) {
        previousRoomId = oldRoomId;
        Room* newRoom = world.getCurrentRoom();
        if (newRoom) std::cout << "Вы перешли в " << newRoom->name << ".\n";
      } else {
        if (!msg.empty()) std::cout << msg << "\n";
      }
      break;
    }

    case Command::TAKE: {
      if (cmd.args.empty()) {
        std::cout << "Что взять?\n";
        break;
      }
      std::string fullName;
      for (size_t i = 0; i < cmd.args.size(); ++i) {
        if (i > 0) fullName += " ";
        fullName += cmd.args[i];
      }
      auto it = std::find_if(room->items.begin(), room->items.end(),
                             [&](const Item& i) { return i.name == fullName; });
      if (it == room->items.end()) {
        for (auto& i : room->items) {
          if (i.name.find(fullName) == 0) {
            it =
                std::find_if(room->items.begin(), room->items.end(),
                             [&](const Item& ii) { return ii.name == i.name; });
            break;
          }
        }
      }
      if (it != room->items.end()) {
        if (it->type == ItemType::TERMINAL) {
          std::cout << "Невозможно подобрать терминал. Используйте "
                       "'использовать терминал'.\n";
          break;
        }
        if (player.pickUpItem(*it)) room->items.erase(it);
      } else {
        std::cout << "Здесь нет такого предмета. Предметы: ";
        for (auto& i : room->items) std::cout << i.name << " ";
        std::cout << "\n";
      }
      break;
    }

    case Command::USE: {
      if (cmd.args.empty()) {
        std::cout << "Что использовать?\n";
        break;
      }
      std::string itemName = cmd.args[0];
      if (cmd.args.size() > 1) itemName += " " + cmd.args[1];

      if (itemName == "карта" || itemName == "Карта") {
        if (player.hasItem("карта") || player.hasItem("Карта")) {
          std::cout << "Доступные выходы: ";
          for (auto& exit : room->exits) std::cout << exit.first << " ";
          std::cout << "\n";
        } else {
          std::cout << "У вас нет карты.\n";
        }
        break;
      }

      if (itemName.find("терминал") != std::string::npos) {
        if (!room->hasTerminal) {
          std::cout << "Здесь нет терминала.\n";
          break;
        }
        std::string terminalId = room->terminalId;
        auto termIt = itemTemplates.find(terminalId);
        if (termIt == itemTemplates.end()) break;
        Item& term = termIt->second;

        if (terminalId == "терминал_кабинет") {
          std::cout << term.description << "\n";
        } else if (terminalId == "терминал_самоуничтожения") {
          if (selfDestructActive) {
            std::cout << "Система самоуничтожения уже активирована.\n";
            break;
          }
          std::cout << "Введите код самоуничтожения: ";
          std::string code;
          std::getline(std::cin, code);
          if (code == "1998") {
            selfDestructActive = true;
            selfDestructTimer = 10;
            std::cout << "\033[1;31mСистема самоуничтожения активирована! У "
                         "вас 10 ходов, чтобы покинуть комплекс.\033[0m\n";
          } else {
            std::cout << "Неверный код.\n";
          }
        }
        break;
      }

      player.useItem(itemName);
      break;
    }

    case Command::DROP: {
      if (cmd.args.empty()) {
        std::cout << "Что выбросить?\n";
        break;
      }
      std::string fullName;
      for (size_t i = 0; i < cmd.args.size(); ++i) {
        if (i > 0) fullName += " ";
        fullName += cmd.args[i];
      }
      auto it = std::find_if(player.inventory.begin(), player.inventory.end(),
                             [&](const Item& i) { return i.name == fullName; });
      if (it == player.inventory.end()) {
        for (auto& i : player.inventory) {
          if (i.name.find(fullName) == 0) {
            it =
                std::find_if(player.inventory.begin(), player.inventory.end(),
                             [&](const Item& ii) { return ii.name == i.name; });
            break;
          }
        }
      }
      if (it != player.inventory.end()) {
        std::string itemName = it->name;
        player.removeItem(itemName);
        if (!player.equippedWeaponName.empty() &&
            player.equippedWeaponName == itemName) {
          player.equippedWeaponName.clear();
        }
        std::cout << "Вы выбросили " << itemName << ".\n";
      } else {
        std::cout << "У вас нет такого предмета.\n";
      }
      break;
    }

    case Command::ATTACK:
      std::cout << "Бой начинается автоматически при встрече с врагами.\n";
      break;

    case Command::LOOK:
      break;

    case Command::INVENTORY: {
      std::cout << "\n\033[1;35m=== ИНВЕНТАРЬ ===\033[0m\n";
      std::cout << "Здоровье: " << player.health << "/" << player.maxHealth;
      if (!player.equippedWeaponName.empty())
        std::cout << " | Оружие: " << player.equippedWeaponName;
      std::cout << " | Патроны 9мм: " << player.ammo9mm
                << " | Патроны 12 кал.: " << player.ammo12g << "\n";
      std::cout << "Предметы (" << player.inventory.size() << "/8): ";
      for (auto& it : player.inventory) std::cout << it.name << " ";
      std::cout << "\n";
      break;
    }

    case Command::HELP:
      showHelp();
      break;

    case Command::QUIT:
      isRunning = false;
      break;

    default:
      std::cout << "Неизвестная команда. Введите 'помощь'.\n";
  }
}

void Game::combat(Enemy& enemy) {
  if (!enemy.isAlive()) return;
  while (player.health > 0 && enemy.isAlive()) {
    if (player.poisonTurns > 0) {
      player.health = std::max(0, player.health - 5);
      std::cout << "\033[1;35mЯд наносит вам 5 урона. HP: " << player.health
                << "/" << player.maxHealth << "\033[0m\n";
      player.poisonTurns--;
      if (player.health <= 0) break;
    }

    std::cout << "\033[1;37mВаши действия: 1. Атака  2. Предмет  3. "
                 "Бежать\033[0m\n> ";
    std::string action;
    std::getline(std::cin, action);
    if (action == "1") {
      if (!player.equippedWeaponName.empty() && !player.hasAmmoForWeapon()) {
        std::cout << "Нет патронов для " << player.equippedWeaponName << "!\n";
        continue;
      }
      int dmg = player.getWeaponDamage();
      std::cout << "Вы атакуете на " << dmg << " урона.\n";
      enemy.takeDamage(dmg);
      if (!player.equippedWeaponName.empty()) player.consumeAmmo();
      if (enemy.isAlive()) enemy.attack(player);
    } else if (action == "2") {
      std::cout << "Что использовать? ";
      std::string itemName;
      std::getline(std::cin, itemName);
      player.useItem(itemName);
      if (enemy.isAlive()) enemy.attack(player);
    } else if (action == "3") {
      if (previousRoomId.empty() || previousRoomId == world.currentRoomId) {
        std::cout << "Некуда бежать! Вы в тупике.\n";
        continue;
      }
      int chance = rand() % 100;
      if (chance < enemy.escapeChance) {
        std::cout << "Вы сбежали!\n";
        world.currentRoomId = previousRoomId;
        if (world.getCurrentRoom()) {
          std::cout << "Вы вернулись в " << world.getCurrentRoom()->name
                    << ".\n";
        }
        waitForEnter();
        return;
      } else {
        std::cout << "Не удалось сбежать!\n";
        if (enemy.isAlive()) enemy.attack(player);
      }
    }
  }
  if (!enemy.isAlive()) {
    std::cout << "\033[1;32mВы победили " << enemy.name << "!\033[0m\n";
    for (auto& item : enemy.loot) player.pickUpItem(item);
  }
}

void Game::showHelp() {
  std::cout << "\033[1;36mКоманды:\033[0m\n";
  std::cout << "идти [направление] - перемещение\n";
  std::cout << "взять [предмет] - поднять предмет\n";
  std::cout << "использовать [предмет] - использовать или экипировать\n";
  std::cout << "выбросить [предмет] - выкинуть предмет из инвентаря\n";
  std::cout << "осмотреться - описание комнаты\n";
  std::cout << "инвентарь - показать инвентарь\n";
  std::cout << "выход - выйти из игры\n";
  std::cout << "помощь - показать подсказку\n";
}

void Game::showMainMenu() {
  std::cout << "\n\033[1;35m";
  std::cout << "  =================================================\n";
  std::cout << "  ||          P R O J E C T   W E S K E R        ||\n";
  std::cout << "  ||          Текстовая RPG                      ||\n";
  std::cout << "  =================================================\n";
  std::cout << "\033[0m\n";
  std::cout << "\033[1;33m";
  std::cout << "  +-----------------------------------------------+\n";
  std::cout << "  |  1. Новая игра                                |\n";
  std::cout << "  |  2. Выход                                     |\n";
  std::cout << "  +-----------------------------------------------+\n";
  std::cout << "\033[0m\n";
  std::cout << "\033[1;37mВыберите действие: \033[0m";
}

void Game::runGame() {
  bool menuActive = true;
  while (menuActive) {
    system("cls");
    showMainMenu();
    std::string choice;
    std::getline(std::cin, choice);
    if (choice == "1") {
      Game game;
      game.init();
      if (game.isRunning) {
        system("cls");
        game.run();
      }
      menuActive = false;
    } else if (choice == "2") {
      menuActive = false;
    }
  }
}
