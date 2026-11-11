# KrKr2 Emulator — Architecture

> Status: living document (2026-11). Corresponds to the tree as of the core-module decoupling work
> (Phases 1–3 done, see [CORE_MODULE_DECOUPLING.md](CORE_MODULE_DECOUPLING.md)).

The KrKr2 emulator reimplements the **KiriKiri / TVP2** visual novel engine so games shipped as
Windows programs run natively on Windows, Linux, macOS, and Android. It is **not** an x86 emulator:
instead of executing the game's `.exe`, it re-hosts the game's data (scripts, XP3 archives,
assets) on a portable reimplementation of the engine and of the plugin DLLs games depend on.

```
┌────────────────────────────────────────────────────────────────┐
│ platforms/<os>/main.cpp        → cocos2d-x GL window, spdlog   │
│ cpp/core/environ/cocos2d/      → AppDelegate, MainScene, UI    │
│ ┌──────────────────────────────────────────────────────────┐   │
│ │ tTVPApplication::StartApplication (environ/Application)  │   │
│ │  TJS2 engine init → file backends → archives →           │   │
│ │  tvpLoadPlugins() → System init → startup.tjs            │   │
│ └──────────────────────────────────────────────────────────┘   │
│ cpp/core/{tjs2,base,io,storage,archive,visual,sound,movie,…}   │
│ cpp/plugins/* — built-in replacements for KiriKiri plugin DLLs │
└────────────────────────────────────────────────────────────────┘
```

## 1. Repository layout

| Path | Role |
|------|------|
| `platforms/apple/macos/main.cpp` | macOS entry point (also `linux/`, `windows/`, `android/cpp/`) |
| `cpp/core/` | Engine core, split into CMake library modules (below) |
| `cpp/plugins/` | Built-in reimplementations of Windows plugin DLLs (`krkr2plugin` static lib) |
| `cpp/external/` | Vendored minizip, libbpg |
| `ui/cocos-studio/` | UI forms (`.csb`), fonts, `locale/{en_us,zh_cn}.xml` |
| `tools/xp3/` | Standalone XP3 extractor; links only `tjs2 + core_io + core_storage_tool` |
| `tests/unit-tests/` | Catch2 tests: `core/tjs2`, `core/movie`, `core/visual`, `plugins` |
| `vcpkg.json` | Dependencies: cocos2dx, ffmpeg, openal-soft, opencv4, spdlog, fmt, catch2, 7zip, libarchive, unrar, oniguruma, boost-*, … |

## 2. Core modules (`cpp/core/`)

| Module (CMake target) | Purpose |
|------------------------|---------|
| `tjs2` | The TJS2 scripting language: compiler (bison grammar in `tjs2/bison/`), VM, `ttstr`/variant types, date/regex. Scripts in `tjs2/script/` |
| `core_io` | `tTJSBinaryStream` + `IFileBackend` abstraction (`StdFileBackend`, `LocalFileBackend`, `MobileFileBackend`) |
| `core_storage` / `core_storage_tool` | VFS on top of archives + local dirs: `StorageIntf`, `UtilStreams`, `ArchiveRegistry` (register-per-format creator functions); `core_storage_tool` is the minimal variant linked by tools |
| `archive_xp3` / `archive_zip` / `archive_7z` / `archive_tar` | Archive format readers (`cpp/core/archive/*`); XP3 is the native KiriKiri format |
| `core_base_module` (`base/`) | Engine interfaces & glue: `ScriptMgnIntf` (script exec), `SysInitIntf`, `SystemIntf`, `EventIntf`, `MsgIntf`, `KAGParser` (KAG scenario tags), `StorageImpl` (in `base/impl/`), `CharacterSet` |
| `core_plugin_module` (`plugin/`) | Plugin service: `ncbind` registration framework + `PluginImpl` (`Plugins.link` dispatch) |
| `core_visual_module` (`visual/`) | Layers, bitmaps, drawing devices, GL renderers (`visual/gl`, `visual/ogl`) |
| `core_sound_module` (`sound/`) | Wave playback, mixing (FMOD/OpenAL paths) |
| `core_movie_module` (`movie/`) | Video playback via FFmpeg (`movie/ffmpeg/`, AE* audio classes) |
| `core_environ_module` (`environ/`) | Platform integration + application: `Application.cpp` (engine bootstrap), `cocos2d/` (AppDelegate, MainScene, key mapping, custom FileUtils), `ui/` (all in-emulator UI forms), `ConfigManager/`, per-OS `Platform` impls (`apple/macos/platform.mm`, `linux/Platform.cpp`, `win32/Platform.cpp`, `android/`), `sdl/`, `impl/` (`TVPWindow`, `SystemControl`), `XP3ArchiveRepack` |
| `core_utils_module` (`utils/`) | Threads, tick counts, MD5, encoding/iconv helpers |
| `core_extension_module`, `core_msg` | Extensions and message strings |

`krkr2core` (INTERFACE) and `krkr2plugin` (STATIC) aggregate everything for the app target.
Historical context: `core_base_module` used to pull in the whole engine; the decoupling refactor
([CORE_MODULE_DECOUPLING.md](CORE_MODULE_DECOUPLING.md)) introduced `core_io`, `core_storage`,
and per-format `archive_*` libraries so tools/tests link only what they need. Some domain
modules still form legacy cycles (Phase 4 pending).

## 3. Boot flow (desktop)

1. `platforms/<os>/main.cpp` — creates spdlog sinks (`core`, `tjs2`, `plugin` loggers), stores
   `argv[1]` as the pre-selected game path, runs `TVPAppDelegate` (cocos2d-x).
2. `TVPAppDelegate::applicationDidFinishLaunching`
   (`cpp/core/environ/cocos2d/AppDelegate.cpp:32`) — creates the GL view (960×640 design
   resolution), initializes `LocaleConfigManager` (UI language), creates `TVPMainScene`, and
   after one frame either opens the game from the startup arg or pushes
   `TVPMainFileSelectorForm` (the game browser).
3. Selecting a game file reaches `TVPMainScene` → `::Application->StartApplication(path)`
   (`cpp/core/environ/cocos2d/MainScene.cpp:650`).
4. `tTVPApplication::StartApplication` (`cpp/core/environ/Application.cpp:298`):
   - `TVPInitScriptEngine()` — boots TJS2 and registers the global native classes
     (`Window`, `Layer`, `System`, `Plugins`, `Storages`, `Timer`, …).
   - `TVPInitEnvironFileBackends()` — desktop: `LocalFileBackend`; mobile: `MobileFileBackend`
     (reads packed resources through cocos2d `FileUtils`).
   - `TVPInitEngineArchiveCreators()` — registers XP3/ZIP/7z/TAR archive creators in the
     `ArchiveRegistry` (tools register only XP3).
   - `TVPInitializeBaseSystems()`, config load (`GlobalConfigManager`, per-game
     `IndividualConfigManager`), async image loader.
   - `tvpLoadPlugins()` — registers/loads built-in plugin modules (below).
   - `TVPSystemInit()`, `TVPSystemControl` (watch timer), then
     `TVPInitializeStartupScript()` — executes the game's `startup.tjs` (from its `data.xp3`
     or loose `data/` folder), which drives everything else (usually `Initialize.ks`/KAG).
5. From here the game runs as pure TJS/KAG on the emulated engine; KAG tags are parsed by the
   built-in KAG parser (`cpp/core/base/KAGParser.cpp`).

Exceptions during startup are caught in `StartApplication`; TJS script errors surface as
`eTJSScriptError` with block name + line, shown via `ShowException` and logged.

## 4. Plugin system (the compatibility surface)

KiriKiri games extend the engine with Windows DLLs, loaded from TJS via
`Plugins.link("Foo.dll")` (and auto-loaded `.tpm` manifests). This emulator replaces dynamic
loading with **static registration**:

- Each built-in plugin translation unit declares its module name up front:
  ```cpp
  #include "ncbind.hpp"
  #define NCB_MODULE_NAME TJS_W("KAGParserEx.dll")   // must match what the game links
  NCB_PRE_REGIST_CALLBACK(InitPlugin_KAGParserEx);   // and/or NCB_REGISTER_CLASS(...)
  ```
- `ncbAutoRegister` (see `cpp/core/plugin/ncbind.hpp:2096`, `ncbind.cpp`) collects these into
  `_internal_plugins`, a map keyed by **lower-cased DLL name**.
- `Plugins.link("foo.dll")` → `TVPLoadPlugin` → `TVPLoadInternalPlugin`
  (`cpp/core/plugin/PluginImpl.cpp:48`) → `ncbAutoRegister::LoadModule`, which runs the module's
  Pre/Class/Post registration callbacks, registering native classes/functions into the TJS global.
  `.tpm` names are rewritten to `.dll` before lookup. Already-registered names are no-ops.
- A miss logs `Loading Plugin: <name> Failed` (spdlog, level error) and the TJS call raises —
  most "game doesn't start" reports end here. The fix is a new/extended built-in plugin under
  `cpp/plugins/` (workflow: `.claude/skills/add-plugin/SKILL.md`).

DLL → implementation mapping: [KRKR_PLUGINS.md](KRKR_PLUGINS.md) lists known plugin DLLs and
their upstream sources; the built-in set currently covers (grep `NCB_MODULE_NAME`):
`KAGParserEx`, `psbfile`, `psd`, `motionplayer`, `emoteplayer`, `fstat`, `json`, `dirlist`,
`csvParser`, `addFont`, `saveStruct`, `varfile`, `getSample`, `getabout`, `TextRender`,
`win32dialog`, `windowEx`, `ScriptsEx`, `wutcwf`, `layerExDraw`, `layerExMovie`, `perspective`,
`fftgraph`, `xp3filter`, `DrawDeviceForSteam`, `krkrsteam`, `steam_api`.

Also notable: `cpp/plugins/steam/` (Steam integration) and `layerex_draw` (with a blend2d
backend). Plugin subprojects have their own docs (`cpp/plugins/motionplayer/docs/`,
`cpp/plugins/psbfile/docs/`).

## 5. Platform & UI layer

- Rendering/windowing: **cocos2d-x** (vcpkg `cocos2dx`), OpenGL. `environ/cocos2d/MainScene.cpp`
  hosts `TVPWindowLayer`s (the emulated TVP windows), the in-game console overlay, and all UI
  forms; `CCKeyCodeConv` maps keys to TVP virtual keys.
- Per-OS code: `environ/{apple/macos,linux,win32,android}` implement `Platform.h`
  (paths, message boxes, IME, memory info, app exit). `environ/sdl/tvpsdl.cpp` holds SDL-derived
  helpers used on desktop.
- UI resources: Cocos Studio `.csb` forms in `ui/cocos-studio/ui/`, strings in
  `ui/cocos-studio/locale/{en_us,zh_cn}.xml`, loaded via `LocaleConfigManager`.
- Android additionally wraps the core in `libkrkr2.so` + JNI (`platforms/android/`), with
  breakpad crash reporting and an APK build via Gradle.

## 6. Configuration, savedata, logs

- Emulator preferences: `TVPGetInternalPreferencePath()` =
  `<cocos2d writable path>/.preference/` (per-OS writable dir) — `GlobalPreference.xml`
  (`GlobalConfigManager`), per-game settings (`IndividualConfigManager`), `recentpath.xml`
  (recent games list).
- Game savedata: written through the VFS into the game's own save directory conventions,
  redirected by the storage layer.
- Logs: spdlog to stdout with three named loggers — `core`, `tjs2` (also receives
  `TVPConsoleLog`, mirrored to the in-game console overlay), `plugin`. Default level `info`
  (set in each platform `main.cpp`; lower it for `debug` spew). An in-emulator debug console
  (`ui/ConsoleWindow.cpp`) shows the same lines.

## 7. Build & CI

- CMake ≥ 3.28 + Ninja + vcpkg (`VCPKG_ROOT`), presets in `CMakePresets.json`:
  `{MacOS,Linux,Windows} {Debug,Release} {Config,Build}`; binaries in `out/<os>/<config>/bin/krkr2/`.
- Android via `platforms/android/gradlew assembleDebug` (custom toolchain
  `cmake/vcpkg_android.cmake`, triplets in `vcpkg/triplets`).
- CI (`.github/workflows/ci.yml`): clang-format-20 check (`cpp/`, `platforms/`) → Linux/Windows
  release builds → Android APK. Tests: Catch2 via ctest (`ENABLE_TESTS`, off for Android/iOS).
- `scripts/check_core_module_deps.py` — parses `cpp/core/*/CMakeLists.txt` link lists and fails
  on new dependency cycles among core targets (run locally; intended for CI per
  CORE_MODULE_DECOUPLING Phase 3).

## 8. Testing

- `tests/unit-tests/core/tjs2` — TJS language/`ttstr` tests.
- `tests/unit-tests/core/movie` — FFmpeg/AE audio tests.
- `tests/unit-tests/core/visual`, `tests/unit-tests/plugins` (e.g. `psbfile-dll`).
- Fixtures under `tests/test_files` (incl. e-mote sample data used by the motionplayer docs).

## 9. Design docs

- [CORE_MODULE_DECOUPLING.md](CORE_MODULE_DECOUPLING.md) — core module layering, cycle
  breaking, the xp3-tool minimal-linking refactor, phase status.
- [GAME_COMPATIBILITY.md](GAME_COMPATIBILITY.md) — diagnosing non-running games.
- [KRKR_PLUGINS.md](KRKR_PLUGINS.md) — plugin DLL source registry.
- [rust/](rust/README.md) — planned incremental Rust layer (`rust/` crates + `bridge/` C ABI),
  psbfile as the pilot module.
