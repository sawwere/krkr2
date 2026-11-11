---
name: debug-game
description: Diagnose why a KiriKiri game fails to run on the krkr2 emulator — read logs, identify missing plugins/APIs/archive issues, and decide the fix layer. Use when a game doesn't start, crashes, or misbehaves.
---

# Debug a non-running game

Read `docs/GAME_COMPATIBILITY.md` for the full model. Workflow:

## 1. Reproduce with logs

Run the game per `.claude/skills/run-game` and capture stdout (`/tmp/krkr2-run.log`).
Work top-down through the failure signatures:

## 2. Failure signatures → cause → fix layer

### A. `Loading Plugin: <name> Failed` (most common)
The game linked a plugin DLL with no built-in replacement.

```bash
# is there a built-in?
grep -rn "NCB_MODULE_NAME" cpp/ | grep -i "<name>"
# where does the original come from?
grep -i "<name>" docs/KRKR_PLUGINS.md
```

- No built-in → implement one: `.claude/skills/add-plugin`.
- Built-in exists but still fails → the game links a *variant* name or loads it before
  `tvpLoadPlugins` auto-registration (check ordering in `cpp/core/plugin/PluginImpl.cpp`).
- Registered but a symbol is missing → the TJS error after load names the class/member; add
  the missing registration to the built-in.

### B. `eTJSScriptError` at `@line` of some block, no plugin error
Missing/incorrect engine API. Extract scripts (`tools/xp3`), open the failing file/line,
identify the global object/method used, then:

```bash
grep -rn "TJS_BEGIN_NATIVE_METHOD_DECL" cpp/core/base | grep -i "<methodName>"
grep -rn "TVPCreateNativeClass_" cpp/core | grep -i "<className>"
```

- Class exists, method missing → add a native method declaration to that `*Intf.cpp`.
- Class missing → decide: engine core (`cpp/core/base/`) or plugin-provided (was it a DLL's
  class? → §A).
- Property/getter differences → check `TJS_BEGIN_NATIVE_PROP_DECL` blocks in the same file.

### C. Fails before any TJS output (archive)
- XP3 open/decrypt error → custom filter/encryption. Check `cpp/plugins/xp3filter.cpp` hooks
  and `tTVPXP3ArchiveExtractionFilter`; try extracting with `tools/xp3` (if the tool also
  fails, the format/decryption needs extending in `cpp/core/archive/xp3/`).
- Storage path problems → `cpp/core/base/impl/StorageImpl.cpp`, file backends in
  `cpp/core/io/`.

### D. Crash / abort (native)
- Reproduce under lldb (`bt`), map the frame to the module table in `docs/ARCHITECTURE.md` §2,
  read that code path.
- Common culprits: visual (layer ops, draw devices), movie (codec paths), threading
  (async image loader), plugin bridge code (ncbind invoke).

### E. Runs but renders/plays wrong
- Wrong visuals/coords → `cpp/core/visual/` (+ `docs/ARCHITECTURE.md` §5 for the layer
  pipeline); for e-mote characters see `cpp/plugins/motionplayer/docs/` first — coordinate and
  matrix pitfalls are documented there.
- No sound/video → `cpp/core/sound/`, `cpp/core/movie/ffmpeg/`; check format support and the
  per-platform audio backend (FMOD/OpenAL paths).
- Encoding mojibake → `CharacterSet` / `utils/iconv`; check the game's expected code page
  (Shift-JIS vs CP936).

## 3. Cross-checking against the original engine

The upstream KiriKiri sources (krkrz on GitHub) are the behavioral reference for engine APIs;
`docs/KRKR_PLUGINS.md` maps each plugin DLL to its original source. Reproduce semantics, not
implementation details.

## 4. After fixing

1. Rebuild (`.claude/skills/build`), rerun the game, confirm clean boot.
2. Add or extend a Catch2 test if you fixed a parseable/algorithmic issue
   (`tests/unit-tests/`, fixtures in `tests/test_files/`).
3. Update `docs/GAME_COMPATIBILITY.md` §6 (known problem areas) and, for newly supported
   games, append to `docs/support_games.txt`.
4. All docs/comments in English.
