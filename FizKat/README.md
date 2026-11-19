# ФизКат — ДЗ №2

Консольный учёт парка кикшеринга.

## Сборка и запуск

```
g++ -std=c++17 -Wall -Wextra *.cpp -o fizkat
./fizkat
```

Команды вводятся по одной на строку, `quit` — выход. Пример из условия выводит ровно то, что указано в задании.

## Файлы

| файл | что внутри |
|---|---|
| [main.cpp](main.cpp) | чтение команд, `catch` и печать `error: ...` |
| [Items.hpp](Items.hpp) / [Items.cpp](Items.cpp) | вся иерархия: `InventoryItem`, `EnergyConsumer`, транспорт, вещи, операторы |
| [Fleet.hpp](Fleet.hpp) / [Fleet.cpp](Fleet.cpp) | парк, витрина, отчёты, таблица корпусов |
| [ServiceCenter.hpp](ServiceCenter.hpp) / [ServiceCenter.cpp](ServiceCenter.cpp) | техосмотр и его журнал |
| [Registry.hpp](Registry.hpp) | шаблон `Registry` |
| [Errors.hpp](Errors.hpp) | исключения |

## Команды

| команда | что делает |
|---|---|
| `add scooter id name energy score` | электросамокат |
| `add bike id name score` | велосипед |
| `add ebike id name energy score` | электровелосипед |
| `add rush id name energy score max_speed` | самокат для опаздывающих (креатив) |
| `add sober id name energy score` | самокат-трезвенник (креатив) |
| `add canteen id name energy score max_speed` | самокат до столовой (креатив) |
| `add helmet id name` | шлем |
| `add dock id name energy` | док-станция |
| `energy` | суммарная энергия парка в сутки |
| `report` | число позиций + энергия |
| `novice` | техника для новичков |
| `list` | всё по инвентарному номеру |
| `focus id` / `show_focus` | витрина («самокат дня») |
| `history` | все, кто был на витрине (удалённые — `removed`) |
| `remove id` | убрать из парка |
| `journal` | журнал техосмотров |
| `ride id time` | взять технику покататься в это время (креатив) |
| `late id building now start` | довезти опаздывающего (креатив) |
| `lunch id now` | доехать до столовой до закрытия (креатив) |
| `quit` | выход |

Время пишется как `9:45`. Транспорт (scooter, bike, ebike, rush, sober, canteen) всегда проходит техосмотр, вещи (helmet, dock) — нет. Техосмотр отклоняет, если в имени есть `broken` в любом регистре.

## Наследование

```
InventoryItem (абстрактный)          EnergyConsumer (абстрактный)
 ├── Transport (абстрактный)                 │
 │    ├── Bike                               │
 │    ├── ElectricScooter ───────────────────┤  множественное
 │    │    ├── RushScooter                   │
 │    │    ├── SoberScooter                  │
 │    │    └── CanteenScooter                │
 │    └── ElectricBike ──────────────────────┤  множественное
 └── Thing (абстрактный)                     │
      ├── Helmet                             │
      └── Dock ──────────────────────────────┘  множественное
```

- `InventoryItem` ([Items.hpp](Items.hpp)) — чисто виртуальные `InventoryId()`, `Name()`, `Describe()`, виртуальный деструктор. Ещё там виртуальные `DailyEnergy()` (по умолчанию 0) и `ForNovice()` (по умолчанию false), чтобы `energy`/`novice` обходили парк через указатель на базис без `dynamic_cast`.
- `EnergyConsumer` — чисто виртуальная `DailyEnergy()`, хранит `energy_` и проверяет, что она ≥ 0.
- `ElectricScooter`, `ElectricBike`, `Dock` наследуются и от инвентаря, и от `EnergyConsumer`. Их `DailyEnergy()` перекрывает одноимённую функцию сразу в обеих базах.
- `virtual` наследование не нужно: `EnergyConsumer` не наследует `InventoryItem`, поэтому ромба нет и общая база не попадает в класс двумя путями.

## Умные указатели

Сделано по уточнению преподавателей:

- `unique_ptr` — новая техника до техосмотра. В [main.cpp](main.cpp) создаётся через `make_unique`, передаётся в `ServiceCenter::Inspect` ([ServiceCenter.cpp](ServiceCenter.cpp)). Если техосмотр бросает исключение, `unique_ptr` уничтожает объект при раскрутке стека и парк не трогается. Если всё ок — владение возвращается и через `std::move` уходит в `Fleet::Add`.
- `shared_ptr` — парк хранит принятые позиции в `std::vector<std::shared_ptr<InventoryItem>>` ([Fleet.hpp](Fleet.hpp)). `unique_ptr` превращается в `shared_ptr` в `Fleet::Add`.
- `weak_ptr` — витрина `focus_` в `Fleet`. Она не владеет техникой, а только смотрит на неё. После `remove` последний `shared_ptr` пропадает, `lock()` возвращает пусто, и `show_focus` печатает `removed`. Если `focus` ещё ни разу не вызывали — печатается `empty`.

`delete` в коде нигде нет.

## Исключения

[Errors.hpp](Errors.hpp): `FleetError : std::runtime_error` — общий базис, от него:

- `InspectionError` — техосмотр не пройден (`inspection rejected`);
- `ArgumentError` — неизвестная команда, не хватает аргументов / лишние, score вне 1..10, energy < 0, пустой id, занятый id, неверное время;
- `NotFoundError` — нет такого id или корпуса;
- `RideError` — поездка невозможна: на шлеме не ездят, трезвенник спит, опоздавшему не помочь, столовая закрыта (см. креатив).

Внутри классов только `throw`, печать `error: ...` — в одном месте, в `catch` в `main`. Чтобы парк не испортился наполовину, в `AddCommand` сначала создаётся объект (конструктор проверяет аргументы), потом проверяется id, потом техосмотр, и только в самом конце `fleet.Add`.

## Перегрузка операторов

- `operator<<(ostream&, const InventoryItem&)` — печатает `id name`;
- `operator<(const InventoryItem&, const InventoryItem&)` — сравнение по инвентарному номеру, используется в `Fleet::List` для сортировки (компаратор `ById` разыменовывает указатели и вызывает `<`);
- `EnergyReport::operator+=(const InventoryItem&)` — накопление отчёта: +1 к числу позиций и + суточная энергия. Используется в `Fleet::Report` для `energy` и `report`;
- `Registry::operator[]` и `operator<<` для `Registry`.

## Шаблон

[Registry.hpp](Registry.hpp): `template <typename T, int Cap> class Registry` — журнал на `Cap` записей: `Add`, `Size`, `operator[]`, `operator<<`.

Используется два раза:
- `Registry<std::string, cJournalSize>` — журнал техосмотров в `ServiceCenter` (команда `journal`);
- `Registry<std::weak_ptr<InventoryItem>, cHistorySize>` — история витрины в `Fleet` (команда `history`).

Печать элемента идёт через шаблонную функцию `PrintEntry<T>`, у которой есть явная специализация для `std::weak_ptr<InventoryItem>`: `weak_ptr` нельзя просто вывести через `<<`, его сначала надо `lock()`, а объект может уже быть удалён — тогда печатается `removed`.

## Креативная часть

Три своих самоката, все наследуются от `ElectricScooter` ([Items.hpp](Items.hpp)). Особое поведение каждого — это переопределение одной из виртуальных функций `InventoryItem`. Базовая версия бросает `RideError`, поэтому команды `ride` / `late` / `lunch` работают через указатель на базис без `dynamic_cast`:

| виртуальная функция | по умолчанию | кто переопределяет |
|---|---|---|
| `CheckRide(now)` | бросает «not a vehicle» | `Transport` (можно всегда), `SoberScooter` (нельзя с 2 до 6) |
| `RushSpeed(meters, minutes_left)` | бросает «not a rush scooter» | `RushScooter` |
| `CanteenSpeed(now)` | бросает «not a canteen scooter» | `CanteenScooter` |

`late` и `lunch` сначала тоже вызывают `CheckRide`.

### Самокат для опаздывающих

**`RushScooter`** — сверхскоростной самокат, который везёт только тех, кто опаздывает на пару. Наследуется от `ElectricScooter` и переопределяет:

- `ForNovice()` — всегда `false`, новичкам не дают, даже если score высокий;
- `RushSpeed(meters, minutes_left)` — скорость ровно такая, чтобы успеть к началу пары: `расстояние / оставшееся время`.

Команда `late id building now start`, например `late R-1 GK 9:40 9:45`:
- расстояние от общаг до корпуса берётся из таблицы в начале [Fleet.cpp](Fleet.cpp) (GK, LK, NK, KPM, Arktika, Cifra, KSP; расстояния примерные);
- если пешком (5 км/ч) студент успевает — `error: you are not late, just walk`;
- если нужная скорость больше `max_speed` самоката — `error: too late even for ...`;
- если пара уже началась — `error: the class has already started, ...`;
- иначе печатается скорость: `R-1 Molniya -> GK: 1100 m in 5 min, speed 13.2 km/h`.

### Самокат-трезвенник

**`SoberScooter`** с 2:00 до 6:00 не выдаётся: `CheckRide` бросает `error: Trezvy is sleeping, no rides from 2:00 to 6:00`. В остальное время это обычный электросамокат, в том числе для новичков. Команда `ride T-1 3:30`. Границы задаются константами `cNightStart`, `cNightEnd`.

### Самокат до столовой

**`CanteenScooter`** едет тем быстрее, чем ближе закрытие столовой (`cCanteenClose` = 17:00). Если до закрытия больше часа, он едет на половине `max_speed`. В последний час скорость линейно растёт до `max_speed`. После закрытия — `error: the canteen is already closed`. Например, при `max_speed` 30: в 12:00 будет 15.0, в 16:30 — 22.5, в 16:59 — 29.8. Команда `lunch C-1 16:30`.

Ещё дополнительно: команды `report`, `journal`, `history`.

## ИИ

- Расскладка по файлам .hpp/.cpp и #pragma once;
- Таблица корпусов и поиск по ней обычным циклом: DistanceTo в начале Fleet.cpp;
- Арифметика в креативной части, то есть формулы в RushSpeed, CanteenSpeed и проверка ночного времени в SoberScooter::CheckRide;
- README как оформленный текст и комментарии.
