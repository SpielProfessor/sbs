# SBS — The Simple Build System

**SBS** is a lightweight build tool for C projects. Inspired by Rust's excellent [`cargo`](https://doc.rust-lang.org/cargo/) system, it provides a simple yet customizable alternative to tools like `make`.

---

## ✨ Features

- **Tiny:** Less than 1000 lines of pure C
- **Simple:** Configure your build with **a single `Buildconf` file**
- **Automatic source discovery:** No need to list files manually
- **Incremental builds:** Only recompiles what’s changed using timestamp checks
- **Sensibly opinionated:** Comes with smart defaults, but stays customizable

---

## 🚧 Warning

> **SBS is in beta.** Some features are still under development. Expect occasional rough edges.

---

## 🔧 Example Usage

```bash
sbs --init myproject     # Create a new project
cd myproject
sbs --build --debug      # Build in debug mode
sbs --run --release      # Run the release binary
```

---

## 🔨 Compilation

SBS currently requires a **GCC-compatible compiler** and a **Unix-like operating system**, and is built using a simple `Makefile`.  
In future versions, SBS will be able to build itself using SBS.

SBS has only one dependency: [`inih`](https://github.com/benhoyt/inih), a minimal INI file parser.

---
