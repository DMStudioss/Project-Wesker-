# Архитектурный проект Project "Wesker"

## 1. Общая архитектура

Игра построена по модульному принципу с чётким разделением ответственности:

- **Game** — управляет жизненным циклом игры, основным циклом, обработкой команд.
- **World** — хранит граф локаций (комнат) и обеспечивает навигацию.
- **Room** — описывает отдельную локацию: название, описание, выходы, предметы, врагов.
- **Entity** — абстрактный базовый класс для всех живых существ (игрок, враги).
  - **Player** — персонаж игрока, инвентарь, взаимодействие с миром.
  - **Enemy** — противник, поведение в бою, лут.
- **Item** — предмет, который можно подобрать/использовать.
- **CommandParser** — преобразует ввод пользователя в команды.
- **FileManager** — загрузка/сохранение данных из внешних файлов (JSON).

Взаимодействие между компонентами показано на диаграмме классов.

---

## 2. Диаграмма классов (Class Diagram)

```mermaid
classDiagram
    class Game {
        -World world
        -Player player
        -CommandParser parser
        -bool isRunning
        +run()
        +processCommand(string cmd)
        +saveGame()
        +loadGame()
    }
    class World {
        -map~string, Room~ rooms
        -string currentRoomId
        +getRoom(string id) Room*
        +movePlayer(string direction) bool
        +getCurrentRoom() Room*
    }
    class Room {
        -string id
        -string name
        -string description
        -map~string, string~ exits
        -vector~Item~ items
        -vector~Enemy~ enemies
        +getDescription() string
        +getExits() map
        +addItem(Item item)
        +removeItem(string itemName)
        +getEnemies() vector
        +removeDeadEnemies()
    }
    class Entity {
        <<abstract>>
        #string name
        #int health
        #int maxHealth
        #int attackPower
        +takeDamage(int amount)
        +isAlive() bool
        +getName() string
        +getHealth() int
    }
    class Player {
        -vector~Item~ inventory
        -int maxWeight
        -int currentWeight
        +pickUpItem(Item item) bool
        +useItem(string itemName) bool
        +attack(Enemy& target)
        +getInventory() vector
    }
    class Enemy {
        -string description
        -Item loot
        +attack(Player& target)
        +getLoot() Item
    }
    class Item {
        -string name
        -string description
        -ItemType type
        -int value
        -int weight
        +use(Player& player) bool
        +getName() string
        +getType() ItemType
    }
    class CommandParser {
        +parse(string input) Command
    }
    class FileManager {
        +loadWorldFromJson(string path) World
        +loadItemsFromJson(string path) vector~Item~
        +loadEnemiesFromJson(string path) vector~Enemy~
        +saveGameState(string path, Player, World)
        +loadGameState(string path) tuple~Player, string~
    }

    Game --> World
    Game --> Player
    Game --> CommandParser
    World --> Room
    Room --> Item
    Room --> Enemy
    Player --|> Entity
    Enemy --|> Entity
    Player o-- Item
    Enemy o-- Item
    FileManager ..> World : uses
    FileManager ..> Player : uses
```mermaid
sequenceDiagram
    participant User
    participant Game
    participant Player
    participant Enemy
    participant Room

    User->>Game: ввод "атаковать зомби"
    Game->>Room: getEnemies()
    Room-->>Game: список врагов
    Game->>Enemy: найти врага по имени
    Game->>Player: attack(enemy)
    Player->>Enemy: takeDamage(attackPower)
    Enemy-->>Player: оставшееся здоровье
    alt враг ещё жив
        Game->>Enemy: attack(player)
        Enemy->>Player: takeDamage(attackPower)
        Player-->>Enemy: оставшееся здоровье
    else враг мёртв
        Game->>Room: removeDeadEnemies()
        Game->>Player: pickUpItem(loot)
    end
    Game-->>User: вывод результата боя
