---
name: run-game
description: Launch a KiriKiri game with the krkr2 emulator and capture diagnostic logs (core/tjs2/plugin spdlog output). Use when asked to run a game, reproduce a compatibility issue, or gather logs.
---

# Run a game and capture logs

## Launch

```bash
# macOS (Debug build)
out/macos/debug/bin/krkr2/krkr2.app/Contents/MacOS/krkr2 /path/to/game[data.xp3 | Game.exe | game_dir] 2>&1 | tee /tmp/krkr2-run.log
```

- The path argument is optional: without it, the emulator opens its built-in file selector
  (`MainFileSelectorForm`) and you pick the game interactively.
- Acceptable game targets: the game's `data.xp3`, another `.xp3`, the game directory, or the
  game `.exe` (the exe is only used to locate the game directory — it is never executed).
- Linux binary: `out/linux/debug/bin/krkr2/krkr2`; Windows: `out/windows/debug/bin/krkr2/krkr2.exe`.

## Logs

All logging goes to **stdout** via spdlog with three named loggers:

| Logger | Content |
|--------|---------|
| `core` | engine boot, storage, config |
| `tjs2` | TJS console output (`TVPConsoleLog`), script errors with `@line` + block name |
| `plugin` | plugin loading: `Loading Plugin: <name> Success` / `Failed` |

Default level is `info` (set in `platforms/<os>/main.cpp`). For verbose plugin/engine detail,
temporarily change `spdlog::set_level(spdlog::level::info)` → `debug` and rebuild, or set the
level at the top of the run by editing main.cpp (revert after).

In-emulator equivalents (no terminal needed): the console overlay window shows `TVPConsoleLog`
lines; `outputlog` per-game preference toggles it.

## Useful companion run

Extract the game's scripts to inspect what it links/needs:

```bash
./out/macos/debug/bin/xp3 --output /tmp/game-extracted /path/to/data.xp3
grep -rn "Plugins.link" /tmp/game-extracted | sort -u
```

## What a healthy boot looks like

1. `core` logs engine/system init lines, no errors.
2. Every `Loading Plugin: <name>` line is followed by `Success` (or the name is a no-op repeat).
3. `tjs2` shows the game's own console output (many engines print boot banners).
4. Window shows the game's title/menu. If instead you get a message box with a TJS error, go to
   `.claude/skills/debug-game`.

## Notes

- Emulator preferences live in `<writable path>/.preference/` (`recentpath.xml`,
  `GlobalPreference.xml`, per-game settings) — delete them to reset a broken UI state.
- To debug crashes: `lldb -- out/macos/debug/bin/krkr2/krkr2.app/Contents/MacOS/krkr2 <game>`
  and `bt` on stop.
