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
```

---

## 3. Диаграмма последовательности боя (Sequence Diagram)

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
```

---

## 4. Диаграмма активности игрового цикла (Activity Diagram)

```mermaid
flowchart TD
    Start([Запуск игры]) --> Load[Загрузка мира из JSON]
    Load --> Init[Инициализация игрока]
    Init --> Loop{Игра активна?}
    
    Loop -->|Да| Show[Показать текущую комнату]
    Show --> CheckEnemies{Есть враги?}
    CheckEnemies -->|Да| Combat[Режим боя]
    Combat --> After{Игрок жив?}
    After -->|Нет| GameOver[Конец игры]
    After -->|Да| CheckWin{Условие победы?}
    CheckWin -->|Да| Win[Победа]
    CheckEnemies -->|Нет| Wait[Ожидание команды]
    
    Wait --> Parse[Парсинг команды]
    Parse --> Cmd{Тип команды}
    
    Cmd -->|Перемещение| Move[Попытка перемещения]
    Move --> MoveValid{Переход возможен?}
    MoveValid -->|Да| ChangeRoom[Сменить комнату]
    MoveValid -->|Нет| Error[Сообщение об ошибке]
    ChangeRoom --> Loop
    Error --> Loop
    
    Cmd -->|Взять| Take[Добавить предмет в инвентарь]
    Take --> Loop
    
    Cmd -->|Использовать| Use[Применить предмет]
    Use --> Loop
    
    Cmd -->|Инвентарь| Inv[Показать инвентарь]
    Inv --> Loop
    
    Cmd -->|Выход| Quit[Завершить игру]
    
    Loop -->|Нет| Save[Автосохранение]
    Save --> Exit([Выход])
    GameOver --> Exit
    Win --> Exit
    Quit --> Exit
```

---

## 5. Состояние мира (World State)

Мир представляет собой направленный граф, где вершины — комнаты, рёбра — переходы по сторонам света. Состояние мира включает:

- Идентификатор текущей комнаты игрока.
- Для каждой комнаты:
  - Список оставшихся предметов (после того, как игрок что-то забрал).
  - Список живых врагов (после боёв враги удаляются из комнаты).
- Глобальные флаги (например, «найден ключ-карта», «босс побеждён») — реализуются через наличие определённых предметов в инвентаре или через отдельную систему переменных.

Сериализация состояния выполняется в JSON-файл при сохранении, десериализация — при загрузке.

---

## 6. Алгоритмы ключевых механик

### 6.1. Парсинг команд

Используется словарь синонимов. Входная строка разбивается на слова. Первое слово сопоставляется с глаголом (move, take, use, attack, look, inventory, quit). Остальные слова — аргументы (направление, имя предмета/врага).

```cpp
struct Command {
    enum Type { MOVE, TAKE, USE, ATTACK, LOOK, INVENTORY, QUIT, UNKNOWN };
    Type type;
    std::vector<std::string> args;
};

Command CommandParser::parse(const std::string& input) {
    // разбить строку, привести к нижнему регистру
    // сопоставить первое слово с глаголом
    // вернуть структуру Command
}
```

### 6.2. Боевая система

Пошаговый бой. Игрок атакует первым (если не указано иное). Расчёт урона:

```
damage = max(1, attacker.attackPower)
```

Здоровье цели уменьшается на `damage`. Если здоровье ≤ 0, цель погибает. Если после атаки игрока враг жив, он атакует в ответ.

Во время боя игрок может использовать предметы (например, аптечку) вместо атаки.

### 6.3. Инвентарь и ограничение по весу

Каждый предмет имеет вес. При попытке подобрать предмет проверяется, не превысит ли суммарный вес инвентаря `maxWeight`. Если превышает — игрок получает сообщение и предмет остаётся в комнате.

Использование расходуемого предмета удаляет его из инвентаря и применяет эффект (например, `health += 30`).

### 6.4. Сохранение и загрузка

Формат файла сохранения (JSON):

```json
{
  "player": {
    "name": "Оперативник",
    "health": 80,
    "maxHealth": 100,
    "attackPower": 15,
    "inventory": [
      { "name": "Аптечка", "type": "consumable", "value": 30, "weight": 1 },
      { "name": "Ключ-карта", "type": "key", "weight": 0 }
    ],
    "currentWeight": 1,
    "maxWeight": 20
  },
  "world": {
    "currentRoomId": "lab_entrance",
    "rooms": {
      "lab_entrance": {
        "items": [],
        "enemies": []
      },
      "main_hall": {
        "items": ["Записка"],
        "enemies": ["Зомби-охранник"]
      }
    }
  }
}
```

---
