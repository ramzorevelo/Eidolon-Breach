# Eidolon Breach — Design Reference

This is a working C++20 turn-based roguelite used as a model project for the Software Design final project.
Read the source alongside your own codebase; the purpose of this document is to show where each principle
and pattern lives and what problem it addresses.

---

## How to Use This

Start with the source files listed in each section. The `diagrams/` folder has class and sequence diagrams
for every major subsystem. Cross-reference them; the diagrams show structure and the sections below explain
decisions.

**Start here:**

| File | What to look at |
|---|---|
| `src/Actions/IAction.h` | Minimal interface design; ISP in practice |
| `src/Core/IStatusEffect.h` | Separation of lifecycle hooks from stat modification |
| `src/Core/StatusEffectBase.h` | Template Method applied to a lifecycle skeleton |
| `src/Core/EventBus.h` | Typed, scope-cleaned Publish-Subscribe |
| `src/Entities/EnemyRegistry.cpp` | Factory Method: caller receives base pointer, never sees concrete type |
| `src/Battle/BattleState.h` | How context is passed without coupling callers to Battle |

---

## SOLID Principles

### Single Responsibility

Each class has one reason to change.

`CombatUtils::calculateDamage()` is a free function. Changing the damage formula recompiles nothing except
the utility and its callers. A method on `Unit` or `Battle` would couple damage math to an unrelated class.

`BattleState` is a plain context struct. Action and AI logic receive context without depending on the full
`Battle` class.

### Open/Closed

New behavior is added through new classes, not edits to existing ones.

`IAction` is the central example. Each action is its own class. Adding `VentAction`, `LyraUltimateAction`,
or any slot skill required a new file; the battle loop never changed. `IStatusEffect` follows the same shape:
`BurnEffect`, `SlowEffect`, `ShieldEffect`, and `RegenEffect` are each independent. Adding a new status
effect is a new file.

### Liskov Substitution

Every subclass must be usable wherever its base is expected, with no special casing by the caller.

`Slime`, `StoneGolem`, and `VampireBat` are all `Enemy` subclasses. The battle loop and dungeon system work
through `unique_ptr<Enemy>` returned by `EnemyRegistry::create()` and never see the concrete type.
`dynamic_cast` is prohibited in `Core/`, `Entities/`, `Actions/`, and `Battle/`.

### Interface Segregation

Interfaces are minimal.

`IAction` has four methods: `label()`, `isAvailable()`, `execute()`, `getActionData()`. A caller that only
needs cost information reads `getActionData()` without involving `execute()`.

`IStatusEffect` separates stat modification from damage absorption from lifecycle hooks. A non-shield effect
returns `incoming` unchanged from `absorbDamage()` without implementing something it does not own.

### Dependency Inversion

High-level modules depend on abstractions.

`Battle` never constructs a renderer. It receives `IRenderer&` through `BattleState`. The same battle code
runs with `SDL3Renderer` in the shipped game and `NullRenderer` in every unit test. `EnemyRegistry::create()`
returns `unique_ptr<Enemy>`; the dungeon layer that calls it depends on `Enemy`, never on `Slime` or
`VampireBat`.

---

## Design Patterns

All patterns listed here are GoF patterns with concrete implementations in the codebase.

### Strategy

**GoF Behavioral.** Encapsulates a family of algorithms behind a shared interface, making them
interchangeable at runtime.

Five separate Strategy applications appear in the codebase, each solving the same structural problem in a
different subsystem:

| Interface | Context (who holds it) | Variants |
|---|---|---|
| `IAction` | `PlayableCharacter` (ability list) | `BasicAttackAction`, `SkillAction`, `UltimateAction`, `VentAction`, character-specific actions |
| `IAIStrategy` | `Enemy` (owns one via `unique_ptr`) | `BasicAIStrategy`; new AI behaviors are new classes |
| `IStatusEffect` | `Unit` (owns a list via `unique_ptr`) | `BurnEffect`, `SlowEffect`, `ShieldEffect`, `RegenEffect` |
| `IVestige` | `Party` (owns up to five) | `FlameResonanceVestige`, `AttunistGambitVestige`, etc. |
| `ITurnOrderCalculator` | `Battle` | `SpeedBasedTurnOrderCalculator` |

The diagnostic: adding a new variant (action, AI behavior, status effect, vestige, or ordering rule) requires
a new file. No existing class opens.

**Diagrams:** `class_actions_effects.mmd`, `class_entities.mmd`, `class_battle_events.mmd`

---

### Template Method

**GoF Behavioral.** Defines the skeleton of an algorithm in a base class; subclasses override only the steps
that differ.

`StatusEffectBase` is the concrete base. It handles duration management, tag storage, and all lifecycle hook
defaults (`onApply`, `onTick`, `onExpire`, `onRemove`). Concrete effects inherit from it and override only
what they need: `BurnEffect` and `RegenEffect` override `onTick()`; `ShieldEffect` overrides
`absorbDamage()` and `isExhausted()`; `SlowEffect` overrides `modifyStatsPct()` and nothing else.

Without this, every concrete effect would duplicate duration decrement and tag lookup, and a change to the
expiry rule would require editing every effect class individually.

**Diagrams:** `class_actions_effects.mmd`, `seq_status_tick.mmd`

---

### Observer / Publish-Subscribe

**GoF Behavioral (Pub-Sub variant).** Defines a one-to-many notification dependency. In the Pub-Sub
variant, publishers and subscribers have no knowledge of each other; an event bus is the sole coupling point.

`EventBus` is the bus. `Battle` emits typed events (`UnitDefeatedEvent`, `BreakTriggeredEvent`,
`ResonanceFieldTriggeredEvent`) without knowing what listens. `AchievementSystem`, vestiges, and `Dungeon`
subscribe to the event types they care about with lambda handlers. Neither side holds a reference to the
other.

`EventScope::Battle` subscriptions are cleared at battle end via `clearBattleScope()`, preventing vestige
handlers registered at the start of one battle from firing in later battles.

**Diagrams:** `class_battle_events.mmd`, `seq_eventbus.mmd`

---

### Factory Method

**GoF Creational.** Defines a creation interface; the concrete type constructed is decided inside the
factory, hidden from the caller.

Three factories appear, each following the registry-based form:

`EnemyRegistry::create(id)` reads a string ID, dispatches on the `enemyType` field from a JSON-loaded
blueprint, and returns `unique_ptr<Enemy>`. The dungeon layer calls `create("slime_alpha")` and receives an
`Enemy`. It never includes `Slime.h`, `StoneGolem.h`, or `VampireBat.h`.

`CharacterRegistry::create(id, level)` assembles a `PlayableCharacter` from a JSON blueprint, resolving
each ability slot through `AbilityRegistry`.

`AbilityRegistry::create(id)` maps string IDs to stored lambda factories that produce `IAction` instances.

Adding a new enemy type adds a branch in `EnemyRegistry::instantiate()`; the dungeon, the battle loop, and
the dungeon node types are untouched.

**Diagrams:** `class_dungeon_factory.mmd`, `seq_enemy_factory.mmd`

---

### Builder

**GoF Creational.** Separates the construction of a complex object from its representation.

`DungeonBuilder` encapsulates all map construction: floor counts, node type distribution, and seeded RNG.
`Dungeon` calls `buildFloor()` and receives a ready-to-run node chain. The assembly logic is isolated
entirely from the class that runs the dungeon.

**Diagram:** `class_dungeon_factory.mmd`

---

## UI Abstraction: DIP + Dependency Injection

`IRenderer` and `IInputHandler` are pure interfaces injected into game logic via `BattleState`. This is
Dependency Inversion applied through parameter injection, which is a technique rather than a GoF pattern.
Both interfaces are constructed once in `main()` and threaded through the full execution chain.

The practical result: the entire graphics backend was replaced in one commit. Every class under `Core/`,
`Entities/`, `Actions/`, and `Battle/` was untouched because none of them depended on a concrete renderer.

**Diagrams:** `class_ui.mmd`, `seq_player_turn.mmd`

---

## Diagrams

| File | Shows |
|---|---|
| `class_entities.mmd` | Unit hierarchy, PlayableCharacter, Enemy, Party |
| `class_actions_effects.mmd` | IAction hierarchy (Strategy), IStatusEffect and StatusEffectBase (Template Method) |
| `class_battle_events.mmd` | Battle, EventBus, concrete events, AchievementSystem |
| `class_dungeon_factory.mmd` | EnemyRegistry, CharacterRegistry, AbilityRegistry, DungeonBuilder, MapNode hierarchy |
| `class_ui.mmd` | IRenderer, IInputHandler, SDL3 implementations |
| `seq_player_turn.mmd` | Full player turn: Player selects action and target, Battle dispatches through IAction, EventBus emits events |
| `seq_enemy_factory.mmd` | EnemyRegistry factory: dungeon requests, registry dispatches, concrete subclass returned |
| `seq_eventbus.mmd` | Vestige subscribes at battle start, Battle emits, vestige reacts, scope cleared at end |
| `seq_status_tick.mmd` | Template Method tick: Unit iterates effects, StatusEffectBase delegates to BurnEffect |
