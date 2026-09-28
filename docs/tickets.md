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
- **Lighting shader source — resolved on TICKET-004:** raylib's own
  `lighting.vs`/`lighting.fs` example shader pair is used verbatim (loaded via
  `LoadShader("assets/shaders/lighting.vs", "assets/shaders/lighting.fs")`),
  not a hand-written-from-scratch shader. Resolves the open question raised
  when the ticket was drafted.
- **`rlights.h` warning suppression — resolved on TICKET-004:** the vendored
  header (`src/rlights.h:123`, `Light light = { 0 };`) triggers
  `-Wmissing-field-initializers` under this project's `-Wall -Wextra
  -Wpedantic` flags since it's `#include`d directly into `main.cpp`. Fixed
  with a localized `#pragma GCC diagnostic push` / `ignored
  "-Wmissing-field-initializers"` / `pop` wrapped around just that include,
  rather than marking the include path `SYSTEM` in CMake — the `SYSTEM` path
  approach was discussed and deferred as more relevant once Phase 3 brings in
  a real external physics-engine dependency via the same `FetchContent`
  pattern.
- **Background darkening — tried, explicitly not adopted, on TICKET-004:**
  when the lit model looked underwhelming, the developer wanted the overall
  scene darker with brightness concentrated near the light. pm ruled that
  darkening `ClearBackground` alone was in-scope (a one-constant change
  touching AC1's visual check), but making the grid/cube lighting-responsive
  was scope creep (no `Material` on primitive `Draw*` calls; would reopen the
  Scene/Renderer question) — parked for Phase 2+. The developer tried a black
  `ClearBackground` as a test, then decided against scene-wide darkening and
  reverted to `ClearBackground(WHITE)` to close the ticket. This was a
  developer choice against a working option, not an abandonment due to
  difficulty — noted here so it isn't re-litigated later as if it were never
  tried.
- **Aircraft entity structure — resolved on TICKET-005:** architect ruled
  `main.cpp` stays flat, function-level split only, same pattern as
  `SceneAssets`/`UpdateScene` — the aircraft entity (one small struct, one
  input-reading update function, no resource-ordering hazards) introduces
  strictly less complexity than TICKET-004's lighting work did, so it does
  not clear the bar for a `Scene`/`Renderer` split or multiple files. This is
  explicitly not a third parking of the flat-`main.cpp` question — architect
  named the real next triggers to act on instead of re-parking indefinitely:
  **Phase 3** (physics adds a genuinely separate axis of complexity —
  velocity/forces/integration — with a different shape than rendering) and
  **Phase 4** (HUD becomes a second consumer of `Aircraft` state, at which
  point ownership of who may mutate it stops being obvious from proximity
  alone). Either should trigger the split for real; "there's now a struct
  with fields" should not.
  Aircraft state is a **separate `Aircraft` struct**, not folded into
  `SceneAssets` — `SceneAssets` is a GPU-handle bag with no semantic meaning
  of its own, while `Aircraft.position`/`heading` mean something to the
  simulation regardless of rendering (RADAR/guidance/HUD will all read it in
  later phases). Forward rule set now to avoid relitigating later: once a
  real aircraft `.glb` model loads, its `Model` handle belongs in the
  GPU-handle bag, never in `Aircraft` — `Aircraft` stays pure domain state
  indefinitely, never owning or reading rendering resources.
  Keyboard input is read and applied via a dedicated `UpdateAircraft(Aircraft&)`
  function, parallel to `UpdateScene`, not inlined in `main()` — multi-key
  gameplay input mutating a domain object every frame is a distinct update
  phase, unlike the existing one-line `KEY_Z` camera reset, which stays an
  inline one-off. Loop order: `UpdateCamera` → `UpdateAircraft` →
  `UpdateScene` → draw. `DrawScene`'s signature extends to also take
  `const Aircraft&` rather than introducing a parallel `DrawAircraft`
  function for one primitive shape.
  Non-architectural implementation note carried over from the same review:
  multiply movement deltas by `GetFrameTime()` in `UpdateAircraft` for
  frame-rate-independent movement. How to orient a primitive by
  heading/pitch in Raylib is flagged as a rendering-mentor question for
  implementation time, not decided here.
- **Camera input handling — resolved on TICKET-005:** raylib's `CAMERA_FREE`
  mode was found to hardcode its own keyboard bindings non-overridably inside
  `UpdateCamera()` (confirmed against `rcamera.h`, lines 461-511) — W/A/S/D,
  arrow keys, Q/E, Space/Ctrl — all colliding with `UpdateAircraft`'s input,
  since both read raw keyboard state independently every frame (pressing `W`
  moved both the aircraft and the free camera at once). Architect first
  proposed dropping `UpdateCamera(&camera, CAMERA_FREE)` for a hand-rolled
  observer-camera function, then withdrew that in favor of a cheaper fix after
  the developer proposed it: gate the two calls with a single modifier key,
  mutually exclusive — `if (IsKeyDown(KEY_LEFT_ALT)) UpdateCamera(&camera,
  CAMERA_FREE); else UpdateAircraft(aircraft);` in place of the current
  back-to-back calls. Reasoning for the reversal: TICKET-005's AC3 only
  requires movement to be observable, not simultaneous with free-look, so
  mutual exclusion satisfies it as cleanly as two concurrent systems; gating
  the call (not individual keys) is necessary and sufficient since
  `UpdateCamera`'s mouse-look/zoom run unconditionally whenever it's called at
  all; and this reuses 100% of raylib's already-correct `CAMERA_FREE` math
  instead of risking a first-pass bug hand-deriving `CameraYaw`/`CameraPitch`/
  `CameraMoveToTarget` calls (degree/radian mismatches, `rotateAroundTarget`
  sign conventions) in rendering, the project's flagged weaker area. A single
  hardcoded `if`/`else` at the call site was judged categorically different
  from the input-focus/mode system already ruled out elsewhere in this same
  review (no new type, no persistent state, nothing to design wrong — a
  branch to delete and replace wholesale when Phase 7 UAV control needs real
  mode-switching, not something to extend or migrate). Also ruled out:
  re-picking aircraft's keys (defers the problem to the next key added in a
  later phase, since `CAMERA_FREE` claims nearly the entire conventional
  keyset). Flagged explicitly as a stopgap for MVP-loop observation, not
  permanent input design — a strong candidate for outright replacement (not
  extension) when the camera-follow/chase-cam ticket happens (deferred since
  TICKET-002) or Phase 4 HUD work gives the camera a more deliberate role.
  This forced a scope-line amendment on TICKET-005 (original text said camera
  stays unchanged) — logged as a bug fix forcing an amendment, not scope
  creep, per pm review: TICKET-005's own AC3 already requires movement keys
  to be verifiable by observation, which the collision broke. Does not reopen
  the camera-follow/chase-cam question, which stays a separate future ticket
  per TICKET-002. Q/E-for-yaw is safe to implement any time once the gate
  lands — `UpdateAircraft` and `UpdateCamera` never execute in the same
  frame, so there's no window where their Q/E readings collide.
- **`UpdateScene`/`UnloadScene` removed — noted on TICKET-005:** both had
  gone empty (no shader/lighting/model resources left to update or unload
  once the car model and its lighting wiring were removed this ticket), and
  their unused `SceneAssets`/`Camera3D` parameters were producing
  `-Wunused-parameter` warnings under this project's `-Wextra`. Deleted
  outright rather than kept as warning-suppressed no-ops. This narrows
  TICKET-004's recorded loop shape (`UpdateCamera` → `UpdateAircraft` →
  `UpdateScene` → draw) — `UpdateScene` no longer exists — and `LoadScene`
  is the only member of that original four-function group still standing,
  now returning an all-defaults `SceneAssets` with nothing to load. If a
  real aircraft model/shader/lighting setup returns in a later ticket,
  `UpdateScene`/`UnloadScene` (or equivalents) get reintroduced then, not
  before — this isn't a standing rule against them existing, just against
  keeping empty stubs around for a shape that has nothing left to do.

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
  (`CAMERA_FREE` itself retained on TICKET-005 — gated behind a held modifier
  key so its calls no longer collide with aircraft input, rather than being
  replaced. See Decisions log.)
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

**Status:** Closed (verified — clean build from scratch produced zero
warnings including confirmation the vendored `rlights.h`
`-Wmissing-field-initializers` warnings are suppressed and the prior
`Camera3D` `-Wextra` warning is gone; developer directly ran the app and
confirmed the model is visibly lit and shading responds to camera movement,
within ticket scope; Esc-key close confirmed working; clean resource
teardown via `UnloadScene` covers both model and shader)

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

**Review history:** pm's first static diagnostic pass and a follow-up
rendering-mentor pass — working through the actual GLSL math against the
real vendored raylib shader source, not just the C++ call sites — both
independently confirmed the lighting shader math itself was correct when the
developer initially found the lit model underwhelming. Root cause was scene
geometry, not a bug: light and camera roughly opposite each other (camera-
facing surfaces getting near-zero diffuse), a `YELLOW` light zeroing the blue
channel, and no distance attenuation (by design, not an oversight, per
ticket scope — this isn't a lighting-design exercise). Separately, pm's
static read confirmed a real ordering bug fix: `model.materials[0].shader =
shader` now runs only after the `shader.id == 0` validity check (previously
ran before it, which would have assigned an invalid shader handle on load
failure). The `LoadScene()` failure path was also fixed — it no longer calls
`CloseWindow()` internally, so `main()` doesn't run the full loop against a
closed window; `main()` now gates with `if (!IsModelValid(scene.model) ||
!IsShaderValid(scene.shader)) { CloseWindow(); return 1; }` immediately after
`LoadScene()` returns, before camera setup. `viewPos` is correctly resolved
via `GetShaderLocation` in `LoadScene()` and pushed every frame in
`UpdateScene` via `SetShaderValue`, and `UnloadScene` unloads both the model
and the shader. A build-warning cleanup pass found 10
`-Wmissing-field-initializers` warnings originating from vendored
`src/rlights.h:123`, compiled under `main.cpp`'s strict flags since it's
directly `#include`d — fixed with a localized `#pragma GCC diagnostic`
push/ignore/pop around just that include (see Decisions log); a fresh
rebuild confirmed zero warnings, and the previously-flagged `Camera3D`
`-Wextra` "missing initializer" warning was reconfirmed gone. The developer
then ran the app directly and confirmed the model is visibly shaded and
shading responds to camera movement, closing out AC1. A scene-wide
background-darkening idea was tried (black `ClearBackground`) and explicitly
not adopted — see Decisions log — reverted to `ClearBackground(WHITE)`
before closing. The ticket's open "exact shader source" question is resolved
by use: raylib's own `lighting.vs`/`lighting.fs` example pair, loaded
verbatim, not a from-scratch shader. Finally, Esc-key close was confirmed
working directly by the developer (raylib's default `exitKey` behavior,
unmodified by this project's code).

## TICKET-005 — Aircraft entity: placeholder model + direct keyboard-driven movement

**Status:** Closed (verified — clean build from scratch produced zero
warnings after the last round of fixes; developer directly confirmed the
Alt-gate behaves correctly, holding `KEY_LEFT_ALT` orbits the camera and
releasing it resumes flying the aircraft; Esc-key close unaffected; a
regression found during closure review — see Review history — was fixed and
re-verified before closing)

**Phase:** 2 — MVP loop

**Goal:** Stand up the first owned, per-frame-updated aircraft entity — an
object with its own position/orientation state that keyboard input changes
every frame — replacing the throwaway cube and car placeholders. This is the
entity TICKET-004's Decisions log flagged as the real trigger for revisiting
whether `main.cpp` stays flat.

**Scope — in:**
- Remove the `RED` placeholder cube and the `race.glb` car model draw/load
  calls — both were explicitly throwaway (the car was "chosen only to get
  reps with model loading," never meant to represent the aircraft).
- Introduce one aircraft placeholder — a primitive shape (e.g. an elongated
  box/wedge) is sufficient; does not need to be a sourced aircraft model yet
  (see open questions).
- The aircraft has its own state — position and heading/orientation — that
  is not hardcoded per frame but owned and mutated over time.
- Direct keyboard input changes that state every frame: e.g. forward/back
  translation and yaw/pitch rotation via WASD or arrow keys. This is
  **kinematic, direct control** — set/adjust position and orientation
  directly from input — not physics. No velocity, acceleration, mass,
  forces, lift, drag, or thrust. That is explicitly Phase 3.
- Ground grid may remain as a spatial reference for judging movement (open
  question below on whether "empty sky" means removing it too).
- Camera stays `CAMERA_FREE`, but its call is now gated behind a held
  modifier key (`KEY_LEFT_ALT`), mutually exclusive with `UpdateAircraft` —
  `if (IsKeyDown(KEY_LEFT_ALT)) UpdateCamera(&camera, CAMERA_FREE); else
  UpdateAircraft(aircraft);` in place of the previous back-to-back calls.
  This is forced by a real key-collision bug — `CAMERA_FREE` hardcodes its
  own non-overridable keyboard bindings inside raylib's function body, which
  collide with every key `UpdateAircraft` reads — not a chase-cam/follow-cam
  design change. The camera remains a free-roaming observer, unchanged in
  behavior while active; only when it's active changes (held Alt, instead of
  always). Camera-follow/chase-cam design stays out of scope, a separate
  follow-on ticket, per TICKET-002. See Decisions log.
- Joystick input is explicitly **not** in this ticket — see scope-out.

**Scope — out:** Any flight physics (lift, drag, thrust, mass, forces,
velocity/acceleration integration — all of Phase 3); altitude/speed limits,
stall behavior, any flight-envelope concept; joystick/gamepad input (own
follow-on ticket — needs its own input-mapping decision and hardware to
verify, shouldn't block this ticket); camera-follow/chase-cam design (own
architect-level decision, own ticket); HUD/instrumentation (Phase 4);
collision, world bounds, terrain; multiple aircraft or any AI/UAV control
(Phase 7); sourcing a polished aircraft model (a primitive placeholder is
enough to prove the entity works — swapping in a real model later is a
cheap, low-risk follow-up, not something to gate this ticket on).

**Acceptance criteria:**
- Running `./build/tac-flight-sim` shows one aircraft placeholder in the
  scene; the old `RED` cube and `race.glb` car are gone.
- The aircraft's position and orientation are held in state that persists
  and is updated frame-to-frame, not recomputed from scratch or hardcoded.
- Pressing movement keys visibly translates/rotates the aircraft placeholder
  in real time, confirmed by observation (camera can be manually orbited via
  `CAMERA_FREE` by holding `KEY_LEFT_ALT`, then released to resume flying the
  aircraft, since camera-follow isn't in scope).
- No physics quantities exist anywhere in this code path — no
  velocity/acceleration/force variables, no integration step. A static read
  should be able to confirm this trivially.
- Clean build, zero warnings; clean close (Esc/close button), no crash.
- Whatever `main.cpp`-structure decision comes out of the required architect
  conversation (stays flat one more time, splits into files, introduces a
  `Scene`/`Renderer`-style split, etc.) is implemented, and the decision plus
  reasoning is recorded in the Decisions log — same as every prior
  structural call in this project.

**Architect conversation — resolved, see Decisions log for full reasoning.**
Summary: `main.cpp` stays flat, function-level split only (no
`Scene`/`Renderer`, no new files) — this entity introduces less complexity
than TICKET-004's lighting work did, so it doesn't clear that bar. Real next
triggers to act on, not re-park: Phase 3 (physics adds a genuinely separate
axis of complexity) or Phase 4 (HUD becomes a second consumer of `Aircraft`
state). Aircraft state is a separate `Aircraft` struct (plain data, no
methods), not folded into `SceneAssets` — `SceneAssets` stays a pure
GPU-handle bag, `Aircraft` stays pure domain state, and this ownership line
holds even after a real aircraft model is loaded (the `Model` handle goes in
the GPU-handle bag, not `Aircraft`). Keyboard input is read/applied via a
dedicated `UpdateAircraft(Aircraft&)` function parallel to `UpdateScene`,
not inlined in `main()`. Loop order: `UpdateCamera` → `UpdateAircraft` →
`UpdateScene` → draw. `DrawScene`'s signature extends to take
`const Aircraft&` rather than a new `DrawAircraft` function.

**Open questions (flagged by pm, not decided):**
- **Placeholder representation — resolved:** started as a `DrawCube`
  fuselage plus two independent `DrawTriangle3D` wing panels (six separately
  hardcoded `Vector3` fields on `Aircraft`), found during review to be more
  than the ticket's own "a primitive shape is sufficient" text called for
  and the direct source of a winding/backface-culling bug and a
  position-desync risk once the aircraft could move. Replaced with a single
  `DrawCube` + `DrawCubeWires` outline pair, both rotated/translated via one
  `rlPushMatrix`/`rlRotatef`(yaw, pitch, roll)/`rlPopMatrix` bracket in
  `DrawScene`, reading `aircraft.position`/`.yaw`/`.pitch`/`.roll` directly —
  no separate wing geometry. `assets/models/race-future.glb` and its texture
  were deleted from the repo rather than kept unused. A real model swap
  remains cheap and out of scope, unchanged from the original call.
- **"Empty sky" and the ground grid — resolved:** grid kept (`DrawGrid(100,
  1.0f)`, widened from the original `25` for more visible travel room) as a
  spatial reference, per pm's original recommendation.
- **Exact key mapping — resolved:** `W`/`S` → `position.z`, `A`/`D` →
  `position.x`, `SPACE`/`LEFT_CONTROL` → `position.y` (translation);
  `UP`/`DOWN` → `pitch`, `LEFT`/`RIGHT` → `roll`, `Q`/`E` → `yaw`
  (rotation) — all direct `+= rate * GetFrameTime()` state mutation in
  `UpdateAircraft`, no rlgl/rendering calls in that function. `Q`/`E` for
  yaw was an architect recommendation, confirmed safe only after the
  Alt-gate landed (see Decisions log — raylib's `CAMERA_FREE` claims `Q`/`E`
  for camera roll internally, which would otherwise have collided).

**Scope-creep watch:** any temptation to make "direct keyboard movement"
feel more like flying — banking that affects turn rate, momentum/coasting
when a key is released, speed-dependent turn radius — is 6DOF/flight-
dynamics thinking creeping in early. Phase 3 exists specifically so that
work gets done once, correctly, against real lift/drag/thrust, not
half-built here as a stopgap. If the aircraft placeholder ends up feeling
stiff and un-plane-like after this ticket, that's expected and correct, not
a bug to fix now.

**Review history:** Extensive rendering-mentor collaboration throughout
implementation, not just after (per this project's standing workflow rule).
Early passes diagnosed a `DrawTriangle3D` backface-culling/winding-order bug
on the original wing-panel placeholder (traced to `rlgl`'s default CCW
front-face convention and a missing `rlgl.h` include for
`rlDisableBackfaceCulling`), then walked through raylib's `rlPushMatrix`/
`rlTranslatef`/`rlRotatef`/`rlPopMatrix` matrix-stack pattern from scratch
(referencing raylib's own `models_rlgl_solar_system.c` example) after an
initial attempt put rotation logic in `UpdateAircraft` instead of
`DrawScene`, mixing update/draw concerns; a second attempt had the
`rlRotatef` angle/axis arguments swapped and the roll/yaw axis pairings
reversed relative to this codebase's `position.z`-is-forward convention —
both fixed and reconfirmed. Separately, a real bug was found mid-ticket:
raylib's `CAMERA_FREE` mode hardcodes its own non-overridable keyboard
bindings inside `UpdateCamera()`, colliding with every key `UpdateAircraft`
reads. architect first proposed replacing `UpdateCamera(CAMERA_FREE)` with a
hand-rolled observer-camera function, then withdrew that recommendation in
favor of the developer's own cheaper counter-proposal — a single
`KEY_LEFT_ALT`-gated mutual exclusion between the two update calls — after
re-confirming it fully satisfies AC3 (observability, not simultaneity) at a
fraction of the implementation risk. pm ruled the resulting scope-line
amendment on this ticket a forced bug fix, not scope creep, and logged it.
pm's closure review caught a real regression before this ticket closed: the
original unconditional `UpdateAircraft(aircraft);` call had been left in
place alongside the new `if`/`else` gate, silently reintroducing the exact
collision the gate was built to remove (and, when the gate wasn't
triggered, running aircraft input twice per frame). Fixed and reconfirmed —
by hand for the Alt-gate behavior, and by an actual from-scratch rebuild,
which caught 4 real `-Wunused-parameter` warnings (on `DrawScene`'s `scene`
parameter and `UpdateScene`/`UnloadScene`'s now-dead parameters) that a
static read alone had explicitly flagged as unable to catch. Warnings fixed
(unnamed unused parameter on `DrawScene`; `UpdateScene`/`UnloadScene`
deleted outright as dead no-op stubs — see Decisions log) and the rebuild
reconfirmed clean. Final state: zero build warnings, all six acceptance
criteria verified — four by direct code/build inspection, two (AC3's
Alt-gate behavior, and general run/close behavior) by the developer's own
hands-on check, consistent with this project's stated preference that
interactive GUI verification stays with the developer rather than being
automated.
