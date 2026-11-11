---
name: add-plugin
description: Implement or extend a built-in replacement for a KiriKiri Windows plugin DLL under cpp/plugins/ (NCB_MODULE_NAME + ncbind registration + CMake). Use when a game fails with 'Loading Plugin: X Failed' or a plugin's API is incomplete.
---

# Add / extend a built-in plugin

Built-in plugins replace Windows plugin DLLs. A plugin is just C++ compiled into `krkr2plugin`
that registers TJS-visible classes/functions under a DLL-name key. There is no dynamic loading.

## 1. Study the contract

- Original plugin source/binary: look it up in `docs/KRKR_PLUGINS.md`.
- The **TJS API surface** (global class names, methods, properties, constants) is the contract.
  Best sources: the original plugin's source, its `.tjs` declaration stubs, or how the game's
  scripts use it (extract with `tools/xp3`, grep the game's `.tjs`/`.ks` files).
- Check it isn't already built-in: `grep -rn "NCB_MODULE_NAME" cpp/ | grep -i <dllname>`.
  Some DLLs have several TUs (e.g. `psbfile.dll`); extend the existing one in that case.

## 2. Write the plugin

Simple single-file plugin — add `cpp/plugins/<Name>.cpp` (many existing ones are 50–300 lines;
`addFont.cpp`, `csvParser.cpp`, `dirlist.cpp` are good templates):

```cpp
#include "ncbind.hpp"

#define NCB_MODULE_NAME TJS_W("MyPlugin.dll")   // exact name games link, case-insensitive

// (optional) module init/term callbacks
static void InitPlugin_MyPlugin() { /* ... */ }
static void TermPlugin_MyPlugin() { /* ... */ }
NCB_PRE_REGIST_CALLBACK(InitPlugin_MyPlugin);
NCB_POST_UNREGIST_CALLBACK(TermPlugin_MyPlugin);

// A TJS native class
class MyUtil {
public:
    static int DoThing(const ttstr &arg) { return /* ... */; }
};

NCB_REGISTER_CLASS(MyUtil) {
    NCB_REGISTER_METHOD(MyUtil, DoThing);          // static method
    // NCB_REGISTER_INSTANCE_METHOD / NCB_REGISTER_PROPERTY / NCB_REGISTER_CONSTANT etc.
    // see cpp/core/plugin/ncbind.hpp for the full macro set
};
```

Larger plugin → subdirectory `cpp/plugins/<name>/` with its own `CMakeLists.txt` +
`add_subdirectory` (see `psbfile/`, `psdfile/`, `layerex_draw/`, `motionplayer/`).

Key macro families (in `cpp/core/plugin/ncbind.hpp`, invocation helpers in `ncb_invoke.hpp`,
examples via `ncb_foreach.inc`): class/instance method & property registration, constructors,
factories, raw callbacks, constants, and pre/post regist callbacks. Everything registered under
the same `NCB_MODULE_NAME` becomes loadable together as one pseudo-DLL.

Registration is lazy: `LoadModule` runs the callbacks only when a game actually links the name
(`cpp/core/plugin/ncbind.cpp`).

## 3. Wire it into the build

Single file: add it to `target_sources` in `cpp/plugins/CMakeLists.txt`.
Subdirectory: `add_subdirectory(<name>)` there, linking `krkr2core`.

## 4. Test

```bash
cmake --build --preset="MacOS Debug Build" --target krkr2plugin krkr2
# unit test if the plugin is logic-heavy: tests/unit-tests/plugins/<name>-dll.cpp
# (pattern: psbfile-dll.cpp — loads the module, exercises the API through TJS)
ctest --test-dir out/macos/debug -R <name> --output-on-failure
```

Then run the affected game (`.claude/skills/run-game`) and confirm the plugin line logs
`Success` and the game passes the point where it previously failed.

## 5. Record it

- `docs/KRKR_PLUGINS.md`: add/point to the source row for the DLL name.
- `docs/GAME_COMPATIBILITY.md` §6 if it unblocks a known-broken game; `docs/support_games.txt`
  if the game now boots.
- Complex plugins get their own `cpp/plugins/<name>/docs/` (English) — see `motionplayer/docs/`
  for the house style.
