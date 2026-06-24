# glint

A learning project to build a JIT-compiled shading language using LLVM, with a software rasterizer as the execution target.

The goal is to deeply understand compiler optimization: IR generation, pass pipelines, custom LLVM passes, and measuring their impact on real shader workloads. Graphics is the substrate — shader throughput is how optimization impact gets measured.

## Status

**Phase 1 complete** — software rasterizer running in real time via SDL2.

- Framebuffer with z-buffer depth testing
- Triangle rasterization using edge functions and barycentric coordinates
- 3D pipeline: Vec3/Vec4, Mat4, MVP transforms, perspective projection
- UV sphere with perspective-correct normal and color interpolation
- Phong lighting (the future JIT swap-in point)
- 4× MSAA with per-sample depth testing and box-filter resolve
- Mouse-driven rotation

**Up next** — shading language frontend (lexer, parser, AST), then LLVM IR codegen and ORC JIT.

## Roadmap

| Phase | Description |
|---|---|
| 1 | Software rasterizer (C++ only) ✓ |
| 2 | Shading language design |
| 3 | Frontend: lexer, parser, AST |
| 4A | Backend: LLVM IR + ORC JIT |
| 4B | Metal backend (MSL) |
| 5 | LLVM optimization experiments + benchmarking |
| 6 | Ray tracer as richer execution target |

## Dependencies

- C++17
- [SDL2](https://libsdl.org/) — window and framebuffer display
- LLVM 17 (Core, ORC JIT, IRBuilder) — not yet used; required for Phase 4A onward

## Build

```sh
cmake -B build && cmake --build build
./build/glint
```

Tested on macOS (Apple Silicon). SDL2 is expected at `/opt/homebrew`.

## Controls

- **Click + drag** — rotate the sphere

## Tech stack

- C++17, CMake
- SDL2 for display
- LLVM 17 for JIT compilation (planned)
- Metal / MSL as an alternate backend (planned)
