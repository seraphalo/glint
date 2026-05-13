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

## Phases

**Phase 1 — Minimal Rasterizer (C++ only, no LLVM)**
- Framebuffer (pixel array in memory)
- Triangle rasterization (edge function or barycentric coordinates)
- Z-buffer (depth testing)
- Output via SDL2
- Hardcoded Phong lighting as a placeholder shader

**Phase 2 — Shading Language Design (no code, design decisions only)**
- Define vertex/fragment shader interface
- Built-in types: `vec2`, `vec3`, `vec4`, `float`
- Built-in functions: `dot`, `normalize`, `clamp`, `mix`
- Control flow: `if`, `for`

**Phase 3 — Frontend: Lexer + Parser**
- Lexer: tokenize source into keywords, identifiers, literals, operators
- Parser: recursive descent, produces an AST
- AST nodes: `BinaryExpr`, `CallExpr`, `VarDecl`, `Block`, etc.
- Pure C++, no LLVM dependency

**Phase 4 — Backend: LLVM IR Generation**
- Walk the AST and emit LLVM IR using `IRBuilder`
- Map `vec3` etc. to LLVM vector types (`<3 x float>`)
- Map built-in functions to LLVM intrinsics or hand-written helpers

**Phase 5 — JIT Execution**
- Compile IR to machine code using LLVM ORC JIT
- Expose shader as a function pointer: `void(*)(FragmentInput*, FragmentOutput*)`
- Rasterizer calls this per-pixel, replacing the hardcoded Phong shader

## Build

`cmake -B build && cmake --build build`

## Naming conventions

| Thing | Convention | Example |
|---|---|---|
| Classes / structs | `PascalCase` | `Framebuffer`, `BinaryExpr` |
| Methods | `camelCase` | `setPixel()`, `clear()` |
| Free functions | `camelCase` | `drawTriangle()`, `packRGB()` |
| Local variables | `snake_case` | `pixel_color`, `edge_val` |
| Member variables | `snake_case` + `_` suffix | `width_`, `pixels_` |
| Constants | `kPascalCase` | `kWidth`, `kMaxDepth` |
| Files | `snake_case` | `framebuffer.h`, `lexer.cpp` |

## Working style

- One concern at a time. Get the rasterizer drawing a triangle before touching the shading language; get the language compiling a constant-color shader before adding features.
- No premature abstractions. Three similar lines beats a templated helper.
- When stuck, the goal is to understand, not just to make it compile. Prefer a short explanation of the underlying concept (e.g. edge functions, SSA, ORC layers) over a black-box fix.
