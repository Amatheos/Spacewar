# Spacewar

A compact, modern C++17 remake of *Spacewar!* — the 1962 PDP-1 classic. Two
ships duel in the gravity well of a central star: thrust, coast the slingshots,
trade torpedoes, and gamble on hyperspace to slip a shot you can't dodge.

<!-- ![Spacewar](data/screenshot.png) -->

## The game

Two players share one keyboard. A star sits at the center, pulling both ships
inward with a softened inverse-square gravity — every maneuver is a fight
against the well as much as against your opponent.

- **Fly.** Rotate and thrust with Newtonian inertia; there's no brake, only
  counter-thrust. The star will happily eat a careless orbit.
- **Fire.** Torpedoes inherit your velocity and kick back a little recoil.
  They're subject to the same gravity, so lead your shots and bend them around
  the star. Firing costs a trickle of score, so spraying isn't free.
- **Jump.** Hyperspace teleports you to a random spot — the escape hatch when
  a torpedo has your name on it. It's a gamble: each jump has a rising chance
  of malfunctioning and destroying you.
- **Score.** Kills score, own-goals (star dives, ramming, hyperspace
  malfunctions, self-kills) cost. When the match clock runs out, the higher
  score wins.

## Controls

| Action        | Needle (P1, cyan) | Wedge (P2, orange) |
| ------------- | :---------------: | :----------------: |
| Rotate left   | `A`               | `←`                |
| Rotate right  | `D`               | `→`                |
| Thrust        | `W`               | `↑`                |
| Fire torpedo  | `Left Shift`      | `Right Shift`      |
| Hyperspace    | `Left Ctrl`       | `Right Ctrl`       |

| Menu / system            | Keys                          |
| ------------------------ | ----------------------------- |
| Navigate                 | `↑ ↓ ← →` or `W A S D`        |
| Select                   | `Enter` / `Space`             |
| Back · pause to menu     | `Esc` / `Backspace`           |
| Toggle hitboxes (debug)  | `F1`                          |

## Building

**Requirements**

- Windows, **x64** (enforced at configure time).
- [CMake](https://cmake.org/) **≥ 3.21**.
- A C++17 toolchain — either **Visual Studio 2022** (MSVC) or **GCC / Clang**
  driven by [Ninja](https://ninja-build.org/).

No dependency setup is needed: dependencies are
vendored under `external/` and built from source.

**Visual Studio 2022**

Open the folder directly (`File ▸ Open ▸ Folder`) and VS picks up
`CMakePresets.json` automatically — pick the *Visual Studio 2022 (x64)* preset
and build. Or from a command prompt:

```powershell
cmake --preset vs2022
cmake --build --preset vs2022-release
```

**Command line (Ninja, any compiler on PATH)**

```powershell
cmake --preset ninja
cmake --build --preset ninja
```

**Run**

The executable lands in the preset's `bin` directory:

- Ninja: `build\ninja\bin\Spacewar.exe`
- VS 2022: `build\vs\bin\Release\Spacewar.exe` (or `Debug\`)

Asset paths are baked in at configure time, so the game reads from the repo's
`data/` folder no matter where you launch the `.exe` from.

## Configuration

- **`data/gameplay_settings.json`** — physics and match tunables (thrust,
  gravity, torpedo life, hyperspace odds, match length, respawn/settle timing,
  …). Edits are hot-reloaded a couple of times a second, so you can tweak
  values with the game running.
- **In-game Settings menu** — master volume, match length, and fullscreen,
  persisted to `data/options.json` (created on first save).

## Third-party

Vendored under `external/`, each under its own license: **GLFW** (windowing),
**glad** (GL loader), **miniaudio** (audio), **rapidjson** (settings I/O), and
**stb** (font rasterization + image loading).
