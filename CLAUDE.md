# glint

A learning project: a minimal software rasterizer in C++ that runs fragment shaders written in a small custom shading language, JIT-compiled via LLVM.

## Goals

- **Rasterizer**: framebuffer, triangle rasterization, z-buffer, SDL2 window output
- **Shading language**: lexer → parser → AST → LLVM IR → ORC JIT
- **Integration**: rasterizer calls JIT-compiled fragment shaders via function pointer at runtime

This is a project for learning, not for shipping. Prefer minimal, readable code over generality or performance.

## Tech stack

- C++17
- LLVM 17 (Core, ORC JIT, IRBuilder)
- SDL2 (window + framebuffer blit)

## Planned layout

```
glint/
  src/
    rasterizer/    # framebuffer, triangle, z-buffer
    shader/        # lexer, parser, AST, codegen
    jit/           # ORC JIT host
    main.cpp       # demo: load shader, render triangle
  shaders/         # example .glint shader sources
  CMakeLists.txt
```

(Layout will evolve as the project grows — update this section when it does.)

## Build

TBD — will use CMake once sources exist. Expected: `cmake -B build && cmake --build build`.

## Working style

- One concern at a time. Get the rasterizer drawing a triangle before touching the shading language; get the language compiling a constant-color shader before adding features.
- No premature abstractions. Three similar lines beats a templated helper.
- When stuck, the goal is to understand, not just to make it compile. Prefer a short explanation of the underlying concept (e.g. edge functions, SSA, ORC layers) over a black-box fix.
