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
