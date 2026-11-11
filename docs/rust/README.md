# KrKr2 Rust Layer

> **Build:** CMake + vcpkg + Corrosion · artifacts in `out/<platform>/<config>/`
> **Principle:** Rust lives in `rust/` · C++ lives in `cpp/` · contracts live in `bridge/`

---

## Reading order

| Order | Document | Contents |
|:-----:|----------|----------|
| 1 | [architecture.md](architecture.md) | Design goals, `rust/` / `bridge/` / `cpp/` layering, multi-crate extension |
| 2 | [ffi.md](ffi.md) | C ABI, Corrosion, cbindgen, memory conventions, tool comparisons |
| 3 | [dependencies.md](dependencies.md) | Shared vcpkg, krkr-sys-*, zlib/lz4 decisions |
| 4 | [build.md](build.md) | `KrkrRust.cmake`, output directories, cbindgen hook |
| 5 | [migration.md](migration.md) | Per-module migration steps, directory-layout comparison |

---

## Module migration documents

| Module | Document | C++ glue | Rust crate |
|--------|----------|----------|------------|
| **psbfile** (first to land) | [modules/psbfile.md](modules/psbfile.md) | `cpp/plugins/psbfile/` | `rust/crates/krkr-psb` |

---

## Source layout (planned)

```text
rust/                         # ★ all Rust sources (no C++)
├── krkr-ffi/
├── krkr-sys/{zlib,lz4,…}
├── crates/krkr-psb/
└── tools/krkr-psb-export/
bridge/include/krkr/          # C ABI headers (cbindgen + common.h)
cpp/plugins/                  # C++ glue (TJS, wrapper classes)
```

Until the Cargo workspace is initialized, the documents in this directory are authoritative.

---

## Related links

| Resource | Description |
|----------|-------------|
| [motionplayer docs index](../../cpp/plugins/motionplayer/docs/README.md) | psbfile's main consumer |
| `vcpkg.json` | Native dependency manifest for the whole project |
