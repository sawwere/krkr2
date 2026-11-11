# Migration Process

<<<<<<<< Updated upstream:.agents/notes/proposed/architecture/2026-11-03-rust-migration.md
Status: proposed

方案材料：本页包含待实施的接口或目录设计，不能作为当前构建与可用功能说明；事实以源码及 docs 为准。

[← 索引](2026-11-03-rust-overview.md)
========
[← Index](README.md)
>>>>>>>> Stashed changes:docs/rust/migration.md

---

## 1. Single-module lifecycle

```text
<<<<<<<< Updated upstream:.agents/notes/proposed/architecture/2026-11-03-rust-migration.md
① 新建 rust/crates/krkr-<m> + bridge 头 + 对应模块迁移 note
② C++ 包装委托 Rust（KRKR_USE_RUST_<M>）
③ 字节级 / 单测对齐
④ 删除 C++ 实现，CMake 去掉重复 find_package
⑤ bridge/manifest.toml 标记 stable
========
① Create rust/crates/krkr-<m> + bridge header + docs/rust/modules/<m>.md
② C++ wrapper delegates to Rust (KRKR_USE_RUST_<M>)
③ Align byte-for-byte / with unit tests
④ Delete the C++ implementation; drop duplicate find_package from CMake
⑤ Mark the module stable in bridge/manifest.toml
>>>>>>>> Stashed changes:docs/rust/migration.md
```

---

## 2. Directory-layout comparison

| Option | Assessment |
|--------|------------|
| `cpp/plugins/*/rust/` | ❌ hard to scale |
| Single crate at repo root | △ no layering |
| **`rust/` + `bridge/` + `cpp/`** | ✅ symmetric with `cpp/`; language ownership is obvious |

---

## 3. Module documents

<<<<<<<< Updated upstream:.agents/notes/proposed/architecture/2026-11-03-rust-migration.md
| 模块 | 文档 |
|------|------|
| psbfile | [modules/psbfile.md](2026-11-03-rust-psbfile.md) |
========
| Module | Document |
|--------|----------|
| psbfile | [modules/psbfile.md](modules/psbfile.md) |
>>>>>>>> Stashed changes:docs/rust/migration.md
