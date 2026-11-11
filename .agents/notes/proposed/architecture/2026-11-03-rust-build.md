# CMake and Build Integration

<<<<<<<< Updated upstream:.agents/notes/proposed/architecture/2026-11-03-rust-build.md
Status: proposed

方案材料：本页包含待实施的接口或目录设计，不能作为当前构建与可用功能说明；事实以源码及 docs 为准。

[← 索引](2026-11-03-rust-overview.md) · 选型见 [ffi.md](2026-11-03-rust-ffi.md)
========
[← Index](README.md) · tool choices: [ffi.md](ffi.md)
>>>>>>>> Stashed changes:docs/rust/build.md

---

## 1. Entry point

```cmake
include(${CMAKE_SOURCE_DIR}/cmake/KrkrRust.cmake)
krkr_rust_init()

krkr_rust_crate(NAME psb CRATE krkr-psb)   # → Krkr::Rust::psb

target_link_libraries(psbfile PRIVATE Krkr::Rust::psb)
target_include_directories(psbfile PRIVATE ${KRKR_BRIDGE_INCLUDE})
```

---

## 2. Output directories

```text
out/linux/debug/
  rust-target/x86_64-unknown-linux-gnu/debug/libkrkr_psb.a
  bridge/include/krkr/psb.h
```

`CARGO_TARGET_DIR=${CMAKE_BINARY_DIR}/rust-target` (not the repository-root `target/`).

---

## 3. KrkrRust.cmake (conceptual)

```cmake
function(krkr_rust_crate NAME CRATE)
  corrosion_import_crate(MANIFEST_PATH rust/Cargo.toml CRATES ${CRATE})
  corrosion_set_env_vars(${CRATE}
    KRKR_VCPKG_INSTALLED=${KRKR_VCPKG_INSTALLED}
    KRKR_BRIDGE_OUT=${CMAKE_BINARY_DIR}/bridge/include
  )
  add_library(Krkr::Rust::${NAME} ALIAS ${CRATE})
endfunction()
```

```rust
// rust/crates/krkr-psb/build.rs
fn main() {
    krkr_ffi::emit_cbindgen("psb");
    println!("cargo:rerun-if-changed=src/ffi.rs");
}
```
