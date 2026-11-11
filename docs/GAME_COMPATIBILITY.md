# Game Compatibility — Diagnosis and Fixes

This is the working guide for the project's main goal: making games that currently fail run
correctly. Use it together with the skills `.claude/skills/debug-game` (triage) and
`.claude/skills/add-plugin` (implementing a missing plugin).

## 1. How compatibility works

A KiriKiri game is (normally) a Windows `.exe` + `data.xp3` (+ possibly extra archives and
plugin DLLs). The emulator ignores the `.exe` entirely and re-hosts the game on its own engine:

1. **Engine core** — reimplemented TVP2: TJS2 VM, KAG parser, layers/visuals, sound, movies.
2. **Plugin DLLs** — reimplemented *statically* inside `cpp/plugins/`. When game scripts call
   `Plugins.link("Foo.dll")`, `ncbAutoRegister::LoadModule` (`cpp/core/plugin/ncbind.cpp:12`)
   looks up a built-in registered under that (lower-cased) name. There is no dynamic loading of
   real Windows DLLs.
3. **Archives** — XP3 (incl. encrypted/filtered variants), ZIP, 7z, TAR via the
   `ArchiveRegistry` (`cpp/core/storage/`); the `xp3filter` built-in handles known XP3
   obfuscation hooks.

A game runs when every engine API and every plugin its scripts touch exists and behaves close
enough to Windows. Failures are therefore almost always: *missing plugin*, *missing/incorrect
native API or class member*, *unsupported archive variant*, or *behavioral difference*
(rendering, coords, timing, encoding).

## 2. Triage workflow (summary)

1. Run the game and capture stdout logs (see `.claude/skills/run-game`). Loggers: `core`,
   `tjs2`, `plugin`. Watch for, in order of frequency:
   - `Loading Plugin: <name> Failed` → missing built-in plugin (§3).
   - `eTJSScriptError` with block name + line → a TJS-level API gap or a plugin symbol the
     game expects but the built-in doesn't export (§4).
   - Archive open/decrypt errors before any script runs → XP3 variant issue (§5).
   - Crash/abort in native code → engine bug; get a backtrace (lldb on macOS) (§7).
2. Identify what the game needs:
   - Grep the game's scripts (extract `data.xp3` with `tools/xp3`) for `Plugins.link(` and
     `Plugins.autoload` usage.
   - Cross-check each DLL against the built-in list (`grep -r "NCB_MODULE_NAME" cpp/`) and
     [KRKR_PLUGINS.md](KRKR_PLUGINS.md) for upstream sources of the original plugin.
3. Fix at the right layer (missing class member → core interface; missing plugin → `cpp/plugins/`;
   engine behavior → the relevant core module), rebuild, retest, and keep notes below updated.

## 3. Missing plugin (`Loading Plugin: X Failed`)

The most common boot-time failure. Games usually link plugins from `startup.tjs` /
`Initialize.ks`, so the failure appears immediately at launch with a script error mentioning
`Plugins.link`.

- Check the built-in registry first: `grep -rn "NCB_MODULE_NAME" cpp/ | grep -i <dllname>`.
- If absent, implement a built-in replacement under `cpp/plugins/` following
  `.claude/skills/add-plugin`. Find the original plugin's source/binary via
  [KRKR_PLUGINS.md](KRKR_PLUGINS.md); its `.tjs`-visible API surface (class names, methods,
  properties) is the contract you must reproduce.
- If present but the game still errors, the built-in is incomplete — compare the game's usage
  (extracted scripts) against the built-in's registrations (`NCB_REGISTER_*` macros).
- Multiple plugins can share one DLL name mapping (e.g. `psbfile.dll` is registered from more
  than one translation unit).

Note: `.tpm` autoload files are treated as plugin-name manifests; the emulator rewrites
`.tpm` → `.dll` and resolves the name the same way (`cpp/core/plugin/PluginImpl.cpp:95`).

## 4. TJS script errors / missing API

`eTJSScriptError` at a specific `@line` of a named block usually means a global class or member
the game expects is missing or has a different signature:

- Global objects/classes are registered by `TVPInitScriptEngine` and friends across
  `cpp/core/base/*Intf.cpp` (e.g. `Window`, `Layer`, `Bitmap`, `System`, `Storages`, `Timer`,
  `Console`, `SaveStruct`, …). Locate the class (`grep -rn "TJS_BEGIN_NATIVE_METHOD_DECL" cpp/core/base/<Area>Intf.cpp`),
  then add/extend the native declaration.
- Some games probe optional features (`if (classFoo !== undefined)`), others hard-fail.
- Behavioral gaps (wrong values, exceptions mid-scene) need comparing against the original
  engine (krkrz sources) and often per-game tweaks; centralize hacks and document them here.

## 5. Archive / XP3 issues

- Extraction works but game won't: check for a custom/encrypted XP3 (segment filters,
  header seeding). Known filters live behind the `xp3filter` mechanism
  (`cpp/plugins/xp3filter.cpp`) and `tTVPXP3ArchiveExtractionFilter` hooks.
- `tools/xp3` (built with the app) extracts any archive the engine can open — useful to
  inspect `data.xp3` contents (`startup.tjs`, `Initialize.ks`, `.tjs` sources) during triage.
- The repack UI (`environ/XP3ArchiveRepack.cpp`, `ui/XP3RepackForm`) exists for mobile
  packaging workflows.

## 6. Known problem areas

| Area | Notes |
|------|-------|
| e-mote / PSB motion (`motionplayer.dll`, `emoteplayer.dll`) | Complex; see `cpp/plugins/motionplayer/docs/` (architecture, PSB structure, matrix pipeline, render tests). Reference scripts: `data/system/AffineSourceMotion.tjs` style games (e.g. NEKOPARA) |
| `psbfile.dll` (M2 PSB format) | Used by e-mote and some engines' packed assets; tests in `tests/unit-tests/plugins/psbfile-dll.cpp`; Rust rewrite designed in `docs/rust/modules/psbfile.md` |
| Video (FFmpeg) | `cpp/core/movie/ffmpeg/`; format/codec gaps show as black video or boot hangs |
| Steam games | `cpp/plugins/steam/`, `DrawDeviceForSteam.dll`, `krkrsteam.dll`, `steam_api.dll` built-ins |
| Text encoding | Shift-JIS/CP936 game scripts handled by `CharacterSet`/iconv utils; mojibake → check code page assumptions in the game's `Config.tjs` handling |
| Legacy module cycles | Don't add new core↔core dependencies; run `scripts/check_core_module_deps.py` |

## 7. Debugging techniques

- Build Debug, run under a debugger (`lldb out/macos/debug/bin/krkr2/krkr2.app/Contents/MacOS/krkr2 <game>`).
- spdlog level: each platform `main.cpp` calls `spdlog::set_level` — switch to `debug`/`trace`
  for plugin loading details (`Loading Plugin: X Success/Failure`) and TJS internals.
- In-emulator console overlay (`ui/ConsoleWindow.cpp`) mirrors `TVPConsoleLog` output —
  useful on devices without a terminal.
- Unit-test-first: for a format/algorithm fix, reproduce with a small Catch2 test and a
  fixture under `tests/test_files` (see the motionplayer render-test docs for the pattern).

## 8. Recording compatibility status

- `docs/support_games.txt` — one game per line (currently minimal). Append confirmed-working
  games with the date.
- When a game needs a new hack or plugin, add a row to §6 above (or the plugin's own docs) so
  the fix is discoverable later.
