# psbfile Module Migration

<<<<<<<< Updated upstream:.agents/notes/proposed/architecture/2026-11-03-rust-psbfile.md
Status: proposed

方案材料：本页包含待实施的接口或目录设计，不能作为当前构建与可用功能说明；事实以源码及 docs 为准。

[← Rust 层索引](2026-11-03-rust-overview.md)
========
[← Rust layer index](../README.md)
>>>>>>>> Stashed changes:docs/rust/modules/psbfile.md

| Item | Path |
|------|------|
| C++ glue | `cpp/plugins/psbfile/` |
| Rust crate | `rust/crates/krkr-psb` |
| C header | `bridge/include/krkr/psb.h` |
| Consumer | `motionplayer` → `PSB::PSBFile` |

<<<<<<<< Updated upstream:.agents/notes/proposed/architecture/2026-11-03-rust-psbfile.md
通用规则见 [../ffi.md](2026-11-03-rust-ffi.md)、[../dependencies.md](2026-11-03-rust-dependencies.md)、[../build.md](2026-11-03-rust-build.md)。
========
General rules: [../ffi.md](../ffi.md), [../dependencies.md](../dependencies.md), [../build.md](../build.md).
>>>>>>>> Stashed changes:docs/rust/modules/psbfile.md

---

## 1. Position in the architecture

```text
cpp/plugins/psbfile/
        │  #include <krkr/psb.h>
        ▼
bridge/include/krkr/psb.h
        ▼
rust/crates/krkr-psb/
```

---

## 2. Responsibility split

| Stays in C++ | Moves to Rust |
|--------------|---------------|
| ncbind, `PSBMedia`, the `PSBFile` shell | Parsing, decompression, chunk tables |
| `types/*` (phase 2) | zlib, RL, Emote LZ4 streams |
| stream → `vector<uint8_t>` | seed, decrypt |

---

## 3. C API

Symbol prefix: **`krkr_psb_`**.

```c
KrkrPsbDoc* krkr_psb_open_memory(const uint8_t* data, size_t len, int32_t seed);
void krkr_psb_close(KrkrPsbDoc* doc);
bool krkr_psb_read_chunk(KrkrPsbDoc* doc, int32_t chunk_id, uint8_t* buf, size_t len);
```

---

## 4. CMake

```cmake
krkr_rust_crate(NAME psb CRATE krkr-psb)
target_link_libraries(psbfile PRIVATE Krkr::Rust::psb)
```

---

## 5. Migration phases

| Phase | Contents | Acceptance |
|-------|----------|------------|
| P0 | workspace + `KrkrRust.cmake` + empty crate | links on all platforms |
| P1 | headers + chunk table | `psbfile-dll` |
| P2 | offset API | `motionplayer-dll` |
| P3 | decompression | byte diff |
| P4 | delete C++ parser | NEKOPARA fixture |
| P5 | CLI, types | JSON diff |

---

## 6. Tests

| Layer | Location |
|-------|----------|
| Rust | `rust/crates/krkr-psb/tests/` |
| C++ | `tests/unit-tests/plugins/psbfile-dll.cpp` |
| Integration | `motionplayer-dll` |
