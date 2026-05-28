# DevLog

## 2026-05-28 packRGB takes 0–255, not 0–1
**Symptom**: White light constant `packRGB(1, 1, 1)` produced near-black output instead of white. Phong shading rendered all surfaces nearly invisible.
**Root cause**: `packRGB` takes `uint8_t` channel values (0–255), not normalized floats (0–1). `packRGB(1, 1, 1)` encoded a color of (1/255, 1/255, 1/255) ≈ pitch black.
**Fix**: Use `packRGB(255, 255, 255)` for white. Added `unpackRGBf`/`packRGBf` helpers (`framebuffer.h`) that convert to/from normalized `Vec3` for shading math.
**Notes**: Whenever a constant looks 256× too dark or too bright, suspect a unit-mismatch between integer-channel and float-channel colors.

## 2026-05-28 Reflection vector formula
**Symptom**: Specular highlight was visually wrong; appeared inverted or in the wrong location as the light moved.
**Root cause**: Wrote `R = (N - L) * 2 * dot(N, L)`, which expands to `2·dot·N − 2·dot·L`. The correct formula is `R = 2·dot(N, L)·N − L` — the `L` term must not be scaled by `2·dot`.
**Fix**: Replaced with `Vec3 R = n * 2 * n_l_dot - light_dir;` in `phongShade` (`triangle.cpp`).
**Notes**: When implementing a textbook formula, don't algebraically rearrange it on the fly — copy it literally first, then simplify if needed.

## 2026-05-28 Cracks between adjacent triangles
**Symptom**: Thin lines of background color visible along shared edges of the UV sphere triangles.
**Root cause**: Edge test used strict `> 0`, so pixels exactly on a shared edge had at least one edge value equal to 0 and were rejected by *both* triangles.
**Fix**: Changed to `>= 0` (`triangle.cpp`). Pixels on a shared edge are now drawn by both adjacent triangles; z-buffer collapses the duplicate writes.
**Notes**: The "proper" fix is the top-left rule — each edge is exclusively assigned to exactly one triangle via a per-edge classification. On the explore list. GPUs do this.

## 2026-05-29 Perspective-correct interpolation: yes for normals/colors, no for z
**Symptom**: Subtle but visible distortion when interpolating per-vertex normals and colors under perspective — barycentric weights computed in screen space don't match world-space proportions.
**Root cause**: Screen-space barycentric weights are distorted by the perspective divide. World-space quantities interpolated with them drift.
**Fix**: For each per-vertex attribute `a`, interpolate `a * (1/w)` using screen-space weights and divide by interpolated `1/w`. Plumbed `inv_w` through `ProjectedVertex` (`pipeline.h`).
**Notes**: NDC `z` (post-perspective-divide, what's stored in the depth buffer) does *not* need correction — it's already naturally linear in screen space. Rule of thumb: things computed *before* the perspective divide need correction, things *after* it don't.

## 2026-05-29 UV sphere poles cause sliver triangles → NaN
**Symptom**: `inv_w_interp` printed as `NaN`; pixels near the poles of the sphere flashed black or contained garbage.
**Root cause**: At the UV sphere poles, all `(0, j)` and `(stacks, j)` vertices collapse to the same point (`sin(0) = 0`). The polar quads produce either fully-degenerate triangles (area = 0) or extremely thin slivers (area ≈ 1e-4 in screen-space pixel units). Dividing edge values by such tiny areas blows up float error.
**Fix**: Early-return when `std::abs(area) < 1e-3f` in `drawTriangle` (`triangle.cpp`). Combined with the existing `area == 0` check.
**Notes**: The real fix is generating the sphere with triangle fans at the poles (one apex vertex + a ring), eliminating the degenerate quads entirely. On the explore list as "UV sphere pole fans".
