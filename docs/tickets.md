# Tickets

Running log of scoped work for tac-flight-sim, kept in the repo (not in any
AI tool's local memory) so it travels across machines via `git clone`. Update
this file when a ticket is drafted, closed, or a project-level decision is
made — see CLAUDE.md for phase/roadmap context and team-role workflow.

## Decisions log

- **Raylib dependency:** pulled in via CMake `FetchContent`, pinned to tag
  `5.5` (not a system package, not vendored). Reasoning: reproducible builds
  across machines/distros, and it sets the pattern to reuse for the physics
  engine dependency in Phase 3.
- **C++ standard:** C++20, set explicitly in `CMakeLists.txt`
  (`CMAKE_CXX_STANDARD 20`, required, extensions off) — not left to compiler
  default.
- **Warnings:** `-Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor` on the
  main target from commit one.
- **Naming — resolved:** the CMake `project()` name was `tac-sim-flight`
  while the repo, executable target, and README used `tac-flight-sim`.
  Flagged during TICKET-001 review; `project()` now also uses
  `tac-flight-sim`, so all four are consistent.
- **`CMAKE_BUILD_TYPE`:** defaults to `RelWithDebInfo` via a guard right
  after `project()` (before `FetchContent_MakeAvailable`), so raylib and the
  app both build with debug symbols instead of silently unset flags. Fixes
  a warning architect traced to raylib's own fallback setting the build type
  to an empty string, not a usable default.
- **`build/` is gitignored**, not committed. It was accidentally tracked for
  two early commits and later untracked (`git rm -r --cached build/`) —
  noted here so it doesn't happen again after a future `git add -A`.
- **`main.cpp` structure — resolved on TICKET-004:** whether to stay flat
  had been explicitly parked twice (after TICKET-002 and TICKET-003).
  Architect's decision: introduce function-level organization only
  (`LoadScene`/`UpdateScene`/`DrawScene`/`UnloadScene`), still a single
  file — no new source files, no classes with behavior. Trigger for this
  was lighting introducing real cross-resource ordering dependencies
  (`Model`, `Shader`, `Light[]`, `Camera3D`) and a genuine new per-frame
  update phase, not file length. The next real structural trigger is
  Phase 2's first owned, per-frame-updated aircraft entity — that's when a
  multi-file split or `Scene`/`Renderer` class becomes a live question
  again, not before.
- **`LoadScene()` return-value construction — resolved on TICKET-004:**
  a single `SceneAssets` value is declared once and built up incrementally
  across the function (model, then shader, then lights), returned early on
  model-load failure and again — more complete — at the end on success.
  Architect's call, over constructing two separate `SceneAssets` values (one
  per branch): the failure and success cases are the same struct at
  different stages of completion, not two distinct outcomes, so one shared
  variable avoids duplicating field assignments as `SceneAssets` gains more
  fields.
- **`BeginMode3D`/`EndMode3D` bracket ownership — resolved on TICKET-004:**
  `main()` keeps the bracket (wrapping the call to `DrawScene`), rather than
  `DrawScene` owning it internally — both were flagged as defensible by
  architect, so this was a developer call, not left to default silently.
  Reasoning: `main()` already owns `DrawFPS`, the only 2D content today, with
  no wrapping function of its own — keeping `main()` as the thing that opens
  and closes each drawing region (3D bracket now, a 2D/HUD region later)
  keeps both regions handled the same way. `DrawScene`'s body is just the
  draw calls (`DrawGrid`/`DrawCube`/`DrawModel`), not the mode bracket.
  Revisit if/when 2D/HUD content grows enough to justify its own ownership
  structure — not before.

## TICKET-001 — Bootstrap project + open a raylib window

**Status:** Closed (verified — clean build ran end-to-end, window opened,
closed cleanly on both close-button and Esc).

**Phase:** 1 — Raylib fundamentals

**Goal:** Get a raylib app building and running from scratch, rendering a
single static frame, to prove the toolchain/build/dependency setup works
before any real scene work starts.

**Scope:** Project structure (`src/`, root `CMakeLists.txt`), raylib via
FetchContent, a flat `main.cpp` that opens a window, runs a game loop,
clears to a solid color every frame, and closes cleanly on close/Esc, plus
`.gitignore` and a README "Build & Run" section. No 3D content, no extra
input handling, no code abstractions.

**Acceptance criteria:**
- One documented build command (`cmake -B build && cmake --build build`)
  produces a runnable binary with no manual steps.
- Window opens at 1280x720, titled "tac-flight-sim".
- Solid-color clear every frame, stable framerate.
- Clean close via close button and Esc, no crash.
- README has an accurate Build & Run section.

**Review history:** four pm passes were needed before closure — empty
`.gitignore`, then a README run-path mismatch, then a CMake
`target_link_libraries` referencing a target name that didn't exist
(would have failed at configure time), then finally a clean pass. Final
closure was confirmed by actually running the build end-to-end, not just a
static read.

## TICKET-002 — 3D scene with Camera3D, ground plane, and placeholder cube

**Status:** Closed (verified — clean build ran end-to-end, window opened,
closed cleanly on both close-button and Esc; scene renders with the camera
moving/orbiting via CAMERA_FREE and the cube visibly resting on the grid).

**Phase:** 1 — Raylib fundamentals

**Goal:** Get a `Camera3D` rendering a basic 3D scene — first real reps with
raylib's 3D pipeline (camera modes, `BeginMode3D`/`EndMode3D`, drawing
primitives) before model loading or lighting.

**Scope — in:**
- Add a `Camera3D` (perspective) to the existing window/loop.
- Draw a ground plane (`DrawPlane` or grid) for spatial reference.
- Draw one placeholder cube (`DrawCube`, primitive — no model file).
- Camera movement/orbit via raylib's built-in `UpdateCamera`.
- Stays flat in `main.cpp` — no new files, no classes/structs.

**Scope — out:** Lighting/shaders, model loading, any aircraft-specific
concepts (flight axes, HUD, aircraft placeholder), custom camera controller,
multiple objects/textures/materials.

**Acceptance criteria:**
- Running `./build/tac-flight-sim` shows a ground plane + cube in 3D
  perspective.
- Camera can be moved/orbited via mouse/keyboard; cube stays visually
  anchored to the plane as the camera moves.
- Clean close (Esc/close button), no crash.
- Same build command, no new CMake dependencies.
- `main.cpp` stays a single file.

**Open questions (flagged by pm, not decided):**
- Camera mode choice — resolved by shipping `CAMERA_FREE` (mouse-look
  fly-cam + WASD/space/ctrl, `DisableCursor()` paired correctly). Revisit if
  an aircraft-follow camera design comes up in Phase 2+ — architect call.
- When flat `main.cpp` stops being appropriate as more tickets stack on top
  (model loading, then lighting) — a real architect question, likely to come
  up on the very next ticket.

**Review history:** rendering-mentor ran three passes — found and fixed a
`camera.up` zero-vector bug (would corrupt the view matrix via a
divide-by-zero in the cross-product normalize), confirmed correct
single-call-per-frame `UpdateCamera` usage and the `CAMERA_FREE` mode choice,
confirmed `BeginMode3D`/`EndMode3D` usage and draw-order (`DrawFPS` after
`EndMode3D`, correct for future HUD-over-3D work) are correct, found and
fixed a cube-position bug (cube was sunk into the ground plane, now anchored
at half its height so it rests on the grid), and flagged non-blocking C99
compound-literal syntax (`(Vector3){...}`) in place of standard C++
brace-init. Developer fixed all remaining compound-literal instances; pm
confirmed by static read that all five acceptance criteria pass and scope
stayed clean (no new files, classes, or later-phase concepts — one small
unrequested `KEY_Z`-resets-camera-target line noted but not treated as scope
creep). Final closure confirmed by an actual clean build + run, which also
surfaced a new non-blocking `-Wextra` "missing initializer" warning on
`Camera3D camera = {0};` — worth a follow-up cleanup pass, not a blocker.

pm recommended routing this ticket through rendering-mentor *while*
implementing, not just after — that recommendation held up well across all
three review passes.

## TICKET-003 — Load and render a single static model (.glb)

**Status:** Closed (verified — clean build from scratch ran end-to-end;
normal run loads model + texture with zero warnings; deliberate path-break
test performed by renaming race.glb, confirmed a clear `ERROR: Model did not
load...` log, clean resource unload, and clean window close with no crash,
then reverted).

**Phase:** 1 — Raylib fundamentals

**Goal:** Get one real first-class model loaded from a file and rendered in
the existing scene — first reps with raylib's model/mesh pipeline and
asset-loading lifecycle, before lighting or any aircraft-specific concepts.

**Scope — in:**
- Add an `assets/models/` directory at repo root (sibling to `src/`, not
  nested inside it); `assets/textures/` also created now as convention even
  though this ticket doesn't need it. `assets/shaders/` and `assets/sounds/`
  are not created yet.
- Source one placeholder `.glb` model (Blender export or a free pack —
  Kenney/Sketchfab) and place it under `assets/models/`. Format is `.glb`
  specifically, not `.obj`/`.mtl` — `.glb` embeds textures, avoiding the
  external-texture-path failure mode where a missing/misnamed texture
  reference silently renders the model gray instead of failing loudly.
- `LoadModel(...)` called after `InitWindow`, `UnloadModel(...)` called
  before `CloseWindow` — this ordering is a correctness requirement (GPU
  resource lifetime tied to the context `InitWindow`/`CloseWindow`
  create/destroy), not a style choice.
- Load path is relative to the process's working directory (e.g.
  `"assets/models/x.glb"` run from repo root) — confirm the documented run
  command in the README still resolves it correctly, since this is a real
  footgun if run from inside `build/` instead.
- Minimal inline validation after load: check something like
  `model.meshCount > 0`; on failure, log a clear message and exit early
  rather than proceeding to draw. No silent garbage-draw path.
- Draw the loaded model in the existing scene, either replacing the
  placeholder cube or alongside it (developer's call at implementation time
  — not an architecturally significant decision).
- Stays flat in `main.cpp` — no new source file, no class/struct/RAII
  wrapper. One model, one call site; that's still true after this ticket.
- Route through rendering-mentor while implementing (model loading + draw
  call usage), not just after.

**Scope — out:** Lighting/shaders (next ticket), multiple models,
animation/animated models, any aircraft-specific concepts (flight axes,
aircraft-shaped placeholder, HUD), any code abstraction beyond current flat
`main.cpp` (that question is explicitly deferred to Phase 2, when a real
owned per-frame-updated entity shows up).

**Acceptance criteria:**
- Running `./build/tac-flight-sim` from the documented run path (repo root)
  shows the loaded `.glb` model rendered in the 3D scene alongside the
  existing ground plane.
- Model loads and unloads with no crash, no leak-shaped warning, and no
  validation-check failure on a normal run.
- Deliberately breaking the model path (rename/move the file) produces a
  loud, clear failure (log message + early exit) instead of a blank/garbage
  draw — verified once, then reverted.
- `assets/models/` (and `assets/textures/`) exist at repo root; README
  build/run instructions still work unmodified or are updated if the run
  path assumption changed.
- `main.cpp` stays a single file; no new classes, structs, or headers
  introduced.
- Same build command as TICKET-001/002, no new CMake dependencies (raylib's
  model loading is already included).

**Open questions (flagged by pm, not decided):**
- Which specific placeholder model/pack to use is still genuinely open —
  rendering-mentor suggested Blender export or Kenney/Sketchfab as sources
  but didn't pick one. Pick anything low-poly and license-clean; it's
  throwaway and gets replaced by an actual aircraft model well after Phase 1.
- Cube replaced vs. cube-plus-model is an implementation-time call, not
  something that needs a decision now.
- The real "when does `main.cpp` stop being flat" question is explicitly
  parked again (per architect, second time now) — it lands with Phase 2's
  owned aircraft entity, not here. **This is the second ticket in a row this
  has been raised and parked.** If it comes up a third time on TICKET-004,
  force the architect conversation before starting that ticket rather than
  parking it again — by then the file will have window setup, camera,
  ground plane, cube, and a loaded model in it.

**Review history:** rendering-mentor found and confirmed fixed an inverted
`meshCount` validation condition (was logging on success, not failure), a
missing exit in the failure branch (now logs via `TraceLog(LOG_ERROR)`,
calls `CloseWindow()`, then returns — nothing downstream reachable), and a
missing `DrawModel` call entirely (model was loaded/unloaded but never
drawn). Separately, a real rendering bug was debugged: the model rendered
solid white instead of its texture. Root cause, confirmed against raylib's
console output and glTF-loading source: the `.glb`'s material references an
external texture file by relative path rather than embedding it (despite
being `.glb` format), and that path resolves relative to the model file's
own directory. The texture was first in the wrong directory
(`assets/textures/` instead of `assets/models/Textures/`), then a
case-sensitivity mismatch (`textures` vs. required `Textures`) was found and
fixed — confirmed via console output showing the texture loading as a real
512x512 texture instead of the 1x1 default fallback. pm's scope check then
found `assets/textures/` (the repo-root convention directory, distinct from
`assets/models/Textures/`) was missing entirely — created with a `.gitkeep`.
Final closure required three live checks, none of which had been done
despite prior static reviews: a clean build from scratch, a normal run
confirming zero warnings, and the ticket's explicit deliberate-path-break
test (renamed `race.glb`, confirmed a clean `ERROR` + resource unload +
window close with no crash, then reverted).

**Note:** `assets/models/race-future.glb` exists in the repo but is never
loaded by any code — flagged by pm as dead weight, not scope creep. Worth a
conscious call later (keep as a future placeholder candidate, or delete).

## TICKET-004 — Basic lighting via shader on the loaded model

**Status:** Open

**Phase:** 1 — Raylib fundamentals (**last ticket in this phase** — see Open
questions for the Phase 2 planning flag)

**Goal:** Get one real pass with raylib's shader-based lighting pipeline — a
lit model instead of raylib's default unlit/vertex-color rendering — closing
out Phase 1's stated goal ("camera, model loading, basic lighting before
touching real systems"). This ticket also resolves the flat-`main.cpp`
question parked twice (TICKET-002, TICKET-003): per architect's decision
(see Decisions log), function-level structure is introduced now, staying in
one file.

**Scope — in:**
- Introduce a plain aggregate struct, `SceneAssets` (GPU handle bag, not a
  domain object — no methods, no behavior), holding `Model model`,
  `Shader shader`, `Light lights[MAX_LIGHTS]` (or the minimal equivalent).
- Split `main()`'s body into named functions by lifecycle phase, still
  inside `main.cpp`:
  - `SceneAssets LoadScene()`
  - `void UpdateScene(SceneAssets&, const Camera3D&)`
  - `void DrawScene(const SceneAssets&, const Camera3D&)`
  - `void UnloadScene(SceneAssets&)`
  - `main()` becomes: `InitWindow` → `LoadScene()` → loop
    `{ UpdateCamera; UpdateScene; Begin/Draw/End }` → `UnloadScene()` →
    `CloseWindow`.
- Load a lighting shader and wire it into the already-loaded model's
  material (`model.materials[0].shader = shader`).
- Create one, or a small fixed number of, lights (the `rlights.h` pattern —
  `CreateLight()` returning a `Light` with shader uniform locations already
  resolved — is the idiomatic raylib approach here, not something to
  reinvent).
- New per-frame update step: push camera position and light data into
  shader uniforms inside `UpdateScene` — a genuinely new UPDATE phase that
  didn't exist in TICKET-001–003, and the reason function-level ordering
  matters now (shader must exist before being assigned to the model's
  material; light uniform locations must be valid before `UpdateScene`
  writes to them).
- Fix the still-open `-Wextra` "missing initializer" warning on
  `Camera3D camera = {};` while camera setup is being touched for this
  ticket anyway.
- Still zero new source files, zero classes with behavior — `SceneAssets`
  is data only.
- **Scope amendment (logged, not a unilateral read):** `src/rlights.h` —
  an unmodified, vendored copy of raylib's own example helper header
  (`CreateLight`/`UpdateLightValues`, gated by `#define
  RLIGHTS_IMPLEMENTATION` in exactly one translation unit, `main.cpp`) — is
  an explicitly allowed exception to "no new headers/source files added"
  below. It's third-party vendored code the developer didn't author, not a
  fragmentation of the project's own logic (which is what that criterion
  was meant to prevent), but pm flagged the acceptance criterion as
  literally worded doesn't carve this out on its own — so it's carved out
  here, explicitly, rather than waved through by a narrow reading.

**Scope — out:** Multiple lights beyond what's needed to demonstrate basic
lighting is working (one, or a small fixed count, is sufficient — this is
not a lighting-design exercise); any new files; any class/object beyond the
plain `SceneAssets` aggregate; a `Scene`/`Renderer` split (explicitly
Phase 2's job, triggered by the first owned per-frame-updated aircraft
entity, not by lighting); any aircraft-specific concepts; any
entity/ownership system; PBR or advanced material work (normal maps, IBL,
etc.) — this is Phong/basic-lit only.

**Acceptance criteria:**
- Running `./build/tac-flight-sim` from the documented run path shows the
  existing `.glb` model visibly shaded by at least one light (a lit
  surface, not flat/unlit vertex color) — verified visually, and by moving
  the camera/light to confirm shading actually responds rather than being
  a baked-in texture effect.
- `main.cpp` is reorganized into `LoadScene`/`UpdateScene`/`DrawScene`/
  `UnloadScene` per the structure above; `main()` itself contains no direct
  GPU resource setup/teardown beyond calling these functions and
  `InitWindow`/`CloseWindow`.
- `SceneAssets` is a plain data aggregate — no member functions, no
  constructors/destructors beyond defaults.
- Clean build produces zero warnings, including confirmation the
  `-Wextra` "missing initializer" warning on `Camera3D` is gone.
- Clean close (Esc/close button), no crash; `UnloadScene` unloads the
  shader as well as the model (shader lifetime tracked the same way model
  lifetime was in TICKET-003 — no leak-shaped warning).
- Still a single file (`main.cpp`); no new headers/source files added,
  **except `src/rlights.h`** (vendored, unmodified — see the explicit
  scope amendment above).

**Open questions (flagged by pm, not decided):**
- **Exact shader source is genuinely open.** raylib ships a basic lighting
  vertex/fragment shader pair (`lighting.vs`/`lighting.fs`) plus
  `rlights.h` in its own examples repo — is that the expected source to
  pull in verbatim, or does the developer want to write a minimal shader
  from scratch as an additional rep? Flag to rendering-mentor while
  implementing.
- Exact light count (1 vs. a small fixed number like 2–4) and light type
  (point vs. directional) is an implementation-time call, not
  architecturally significant — pick whatever most clearly demonstrates
  lighting is working.
- **Phase 2 planning should follow immediately after this ticket closes.**
  This is the last item in Phase 1's roadmap description. Once TICKET-004
  is verified closed, the next pm conversation should be scoping the first
  Phase 2 ticket (MVP loop — one aircraft, empty sky, input/camera), not
  another Phase 1 item.

**Note:** don't let "route through rendering-mentor while implementing"
become optional here the way it arguably was for TICKET-002/003 — this
ticket has more moving parts (shader compilation, uniform wiring, material
assignment order) and more silent-failure risk (per the architect's
TICKET-003 white-model parallel) than either of those.

**Review history:** (pending)
