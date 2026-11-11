# Native Dependency Strategy (vcpkg and krkr-sys)

<<<<<<<< Updated upstream:.agents/notes/proposed/architecture/2026-11-03-rust-dependencies.md
Status: proposed

方案材料：本页包含待实施的接口或目录设计，不能作为当前构建与可用功能说明；事实以源码及 docs 为准。

[← 索引](2026-11-03-rust-overview.md)
========
[← Index](README.md)
>>>>>>>> Stashed changes:docs/rust/dependencies.md

The Rust layer links vcpkg through `krkr-sys-*`, avoiding a world where C++ and Rust each ship
their own zlib/lz4.

---

## 1. Principle: one vcpkg, one link

| Dependency | C++ | Rust |
|------------|-----|------|
| zlib | `ZLIB::ZLIB` | `krkr-sys-zlib` |
| lz4 | `lz4::lz4` | `krkr-sys-lz4` |
| RL, Emote LZ4 streams, etc. | Gradually deleted | **pure Rust** |

**Forbidden:** shipping two copies of zlib inside the same `.so` — C++ zlib plus Rust
`flate2/rust_backend`.

---

## 2. krkr-sys-* implementation

```cmake
corrosion_set_env_vars(krkr_psb
  KRKR_VCPKG_INSTALLED=${_VCPKG_INSTALLED_DIR}
)
```

```toml
[features]
default = ["vcpkg-zlib"]
standalone = ["dep:flate2"]   # standalone CLI only
```

---

## 3. Decision table

| Situation | Choice |
|-----------|--------|
| Already in vcpkg and still used by C++ | `krkr-sys-*` |
| Format is private to us | pure Rust in `rust/crates/...` |
| Standalone CLI | `standalone` feature |

<<<<<<<< Updated upstream:.agents/notes/proposed/architecture/2026-11-03-rust-dependencies.md
PSB 模块细节见 [modules/psbfile.md](2026-11-03-rust-psbfile.md)。
========
For PSB module details see [modules/psbfile.md](modules/psbfile.md).
>>>>>>>> Stashed changes:docs/rust/dependencies.md
