# Eidolon Breach

Turn-based roguelite dungeon crawler built in C++20 with SDL3. Assemble a party of Synchrons, fight through
seeded dungeons, collect Vestiges, and manage a personal Exposure gauge that rewards aggression and punishes
recklessness.

**Status:** Active development. Classic Mode is feature-complete.

---

## Screenshots

*Coming soon.*

---

## Features

- Speed-ordered turn-based combat with affinity weaknesses and Toughness Break
- Resonance Field system with affinity voting and team-level Field Discoveries
- Breach Exposure gauge with threshold events per character
- Seven dungeon node types across procedurally seeded floor maps
- Vestige relic system with per-battle event hooks
- Summon system with player-side Manifestations
- Meta-progression: character levels, Stance crystallization, and Aspect Trees
- Classic and Draft run modes
- SDL3 graphical renderer with panel layout, turn-order strip, and scrollable action log

---

## Dependencies

Managed via vcpkg manifest. No manual installation needed after setup.

| Library | Purpose |
|---|---|
| [SDL3](https://github.com/libsdl-org/SDL) | Window, renderer, input events |
| [SDL3_ttf](https://github.com/libsdl-org/SDL_ttf) | Font rendering |
| [nlohmann-json](https://github.com/nlohmann/json) | Blueprint data loading |
| [doctest](https://github.com/doctest/doctest) | Unit tests |

---

## Building

### Requirements

- Visual Studio 2022, Desktop development with C++ workload
- [vcpkg](https://github.com/microsoft/vcpkg)
- PowerShell Core (`pwsh`) for automatic SDL3 DLL deployment

### First-time vcpkg setup

Open **PowerShell or Command Prompt** and run:

```
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
```

`C:\vcpkg` is the recommended location but any path without spaces works. After `integrate install`,
Visual Studio resolves all vcpkg packages automatically.

### Build steps

1. Clone the repository.
2. Open `EidolonBreach.sln`.
3. Select **x64 | Release** and press **Ctrl+Shift+B**.

Dependencies declared in `vcpkg.json` install on the first build. SDL3 DLLs deploy to the output folder
automatically via the vcpkg applocal MSBuild target.

> **Opening in Visual Studio 2022:** This project was created in Visual Studio 2026 (v145 toolset).
> Right-click the solution in Solution Explorer and choose **Retarget Solution** to switch to v143.

### CMake (Linux / macOS)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

---

## Running

Run from the repository root so `data/` and `ShareTechMono-Regular.ttf` resolve correctly:

```
x64\Release\EidolonBreach.exe
```

### Tests

Set `EidolonBreach_Tests` as the startup project and run, or from the command line:

```
x64\Release\EidolonBreach_Tests.exe
```

---

## Project Structure

```
EidolonBreach/
├── vcpkg.json
├── EidolonBreach.sln
├── data/
│   ├── enemies.json
│   ├── characters.json
│   ├── items.json
│   ├── encounters.json
│   └── summons.json
├── EidolonBreach/src/
│   ├── main.cpp
│   ├── Core/           # Data types, interfaces, event bus, utilities
│   ├── Entities/       # Unit, PlayableCharacter, Enemy, Summon, Party, registries
│   ├── Actions/        # IAction and shared concrete actions
│   ├── Characters/     # Per-character actions and registries
│   │   ├── Lyra/
│   │   ├── Vex/
│   │   └── Zara/
│   ├── Battle/         # Battle loop, BattleState, turn order
│   ├── Dungeon/        # Map, nodes, encounter table, run context
│   ├── Vestiges/       # IVestige and concrete vestiges
│   ├── Items/
│   ├── Summons/
│   ├── Meta/           # MetaProgress, AspectTree
│   └── UI/             # IRenderer, IInputHandler, SDL3 implementations
└── EidolonBreach_Tests/tests/
    ├── Core/
    ├── Entities/
    ├── Actions/
    ├── Battle/
    └── Vestiges/
```

---

## License

[MIT](LICENSE)
