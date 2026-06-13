#include <windows.h>

#include "Game.h"

int main() {
  SetConsoleOutputCP(1251);
  SetConsoleCP(1251);
  Game::runGame();
  return 0;
}
