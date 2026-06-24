# glint

A personal project to build a JIT-compiled shading language using LLVM, with a software rasterizer as the execution target.

## Goals (priority order)

1. **Compiler deep dive** — deeply understand compiler optimization through LLVM: IR generation, pass pipeline, writing custom passes, and measuring their effect
2. **Graphics as substrate** — the rasterizer is the execution target; shader performance is how we measure optimization impact
3. **Metal backend** — eventually compile glint shaders to MSL and run on GPU (after the compiler work is solid)

This is a project for learning, not for shipping. Prefer minimal, readable code over generality or performance.

## Tech stack

- C++17
- LLVM 17 (Core, ORC JIT, IRBuilder)
- SDL2 (window + framebuffer blit)
- Metal / MSL (future, Phase 4B)

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

Done:
- Framebuffer (pixel array in memory)
- Triangle rasterization (edge functions + barycentric coordinates)
- Z-buffer (depth testing)
- SDL2 output + interactive render loop
- 3D pipeline: Vec3/Vec4, Mat4, MVP transforms, perspective projection
- Mouse-driven rotation
- Hardcoded Phong lighting as a placeholder shader (extracted into `phongShade` — the JIT swap-in point for Phase 4A)
- UV sphere generation + rendering
- Perspective-correct interpolation for normals and albedo
- Float screen coordinates for sub-pixel precision

Remaining:
- Back-face culling
- MSAA

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

**Phase 4A — Backend: LLVM IR → ORC JIT**
- Walk the AST and emit LLVM IR using `IRBuilder`
- Map `vec3` etc. to LLVM vector types (`<3 x float>`)
- Map built-in functions to LLVM intrinsics or hand-written helpers
- Compile IR to machine code using LLVM ORC JIT
- Expose shader as a function pointer: `void(*)(FragmentInput*, FragmentOutput*)`
- Rasterizer calls this per-pixel, replacing the hardcoded Phong shader

**Phase 4B — Metal Backend (later)**
- AST → MSL source
- Compile and run via Metal API
- Compare GPU vs CPU shader performance

**Phase 5 — LLVM Optimization Experiments**
- Write custom LLVM passes
- Benchmark against `-O0` / `-O2`
- Measure optimization impact on per-pixel shader throughput

**Phase 6 — Ray Tracer**
- Replace rasterizer with a ray tracer as the execution target
- More complex per-pixel computation = richer target for compiler optimization experiments
- Compare optimization impact on ray tracing vs rasterization workloads

## Ideas to explore

- **Cook-Torrance BRDF** — replace Phong with a physically-based reflectance model
- **Shadow maps** — render scene from light's POV to get a depth buffer, use it to determine shadowed pixels
- **SSAO** — screen-space ambient occlusion as a post-process pass
- **Hardware ray tracing** — hybrid rasterization + limited ray tracing (shadows or reflections only)
- **Bezier curves / patches** — prerequisite for rendering the Utah teapot (defined as bicubic Bezier patches)
- **Top-left rule for triangle edges** — replace strict edge test with top-left rule so each pixel on a shared edge is claimed by exactly one triangle
- **Fixed-point rasterization** — replace float screen coordinates with fixed-point (e.g. 24.8) for sub-pixel precision with exact integer edge math, matching how real GPUs do it
- **UV sphere pole fans** — replace the quad strip at the poles with triangle fans to eliminate degenerate/sliver triangles at the source

### GPU directions

- **Metal graphics** — write a simple "draw a triangle" Metal app to learn the API; natural fit on macOS and aligns with Phase 4B
- **Metal Compute** — non-graphics workload (matrix multiply, reduction) to learn GPGPU
- **Vulkan** — cross-platform, low-level, lots of explicit state. Steep learning curve.
- **WebGPU** — modern, cross-platform via Dawn or wgpu. Simpler than Vulkan.
- **OpenGL** — legacy but easiest to get pixels on screen quickly
- **CUDA** — Nvidia-only GPGPU; most mature ML/HPC ecosystem. Needs an Nvidia GPU or cloud rental.
- **GPU shader IRs** — study SPIR-V (Vulkan/OpenCL), NVPTX (Nvidia, LLVM-based), AIR (Apple, LLVM-based), DXIL (D3D12). Direct bridge to the glint compiler work — could emit LLVM IR → any of these as alternative backends.
- **Real shader compilers** — study Slang, DXC, glslang, SPIRV-Cross to see how multi-target shader compilation works in practice

## Procedural generation track

A learning track exploring procedural content generation. Two distinct entry points into the pipeline — keep them straight, because they serve the project's goals differently:

- **Per-pixel (fragment stage)** — noise textures, escape-time fractals (Julia/Mandelbrot), domain warping. Computed in the per-pixel shading function over a full-screen quad. These double as **rich shader workloads**: implement now as hardcoded C++ shaders (same pattern as `phongShade`), later port to `.glint` once the language exists, and use as optimization targets in Phase 5 — far more compute-dense than Phong, so a better measure of pass impact. This entry point directly serves Goals 1 and 2.
- **Geometry pre-pass** — generates vertices/triangles that feed the existing rasterizer. Heightmap terrain, L-system geometry. This is a **separate graphics side-track**, orthogonal to the LLVM/compiler core. Lower priority than the compiler work; pursue only when it isn't competing with it.

Roadmap (ordered; for each step I implement and you review):

1. Julia/Mandelbrot in the per-pixel shader, full-screen quad, color by iteration count. Validates the per-pixel path. (hardcoded C++ shader, `phongShade` pattern)
2. value/Perlin noise + fBm (multi-octave sum), rendered as grayscale. The workhorse function.
3. fBm as a heightmap → displace a grid → rasterize a terrain mesh. First geometry-gen step.
4. diamond-square terrain, compared against step 3.
5. L-system: grammar rewrite → turtle interpretation → line segments → geometry → rasterize. Focus on the branch stack `[]`.
6. domain warping — warp noise input coords with noise, for richer detail.

**Pacing**: the general collaboration norm now lives in Working style (I implement, you brief + review). One addition specific to this staged roadmap — don't reveal a step's brief until I say I'm starting that step.

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

- **I write the implementation, not you.** Before I start anything non-trivial, give a short conceptual brief plus the key pitfalls, then let me implement it myself. When I'm stuck, guide with hints or questions; only show code if I explicitly ask for it.
- Review what I write for correctness, clarity, and whether I actually understood it — review, don't rewrite.
- One concern at a time. Finish Phase 1 before touching the shading language; get a constant-color shader JIT-compiling before adding optimization passes.
- No premature abstractions. Three similar lines beats a templated helper.
- When stuck, analyze the problem and suggest a concrete next step — don't just make it compile.
- When relevant, explain what's happening at the IR or machine code level.
- Prefer minimal, readable implementations — no over-engineering.
- Don't rewrite things that weren't asked about.

## DevLog Convention

After resolving any non-trivial bug or technical decision, append an entry to `DEVLOG.md`:

```
## [Date] Short title
**Symptom**: What was observed, as specifically as possible.
**Root cause**: The actual problem.
**Fix**: What was done to resolve it.
**Notes**: Lessons worth remembering, or relevant file/line references.
```