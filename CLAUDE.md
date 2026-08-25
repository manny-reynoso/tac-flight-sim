# tac-flight-sim

A 3D flight/combat simulator built solo in C++ with Raylib: flight physics, RADAR, missile lock-on, UAV control, and a HUD. This is a personal portfolio project — the primary technical showcase for a C++ software engineering portfolio.

## Ground rules

- **The developer is the only one who writes code.** Agents plan, review, teach, and critique — they do not implement. If an agent is asked to write full implementations, that's out of scope for it; it should describe the approach instead and hand back to the developer to write it.
- Progress moves through the phases below in order. Don't skip ahead to a later phase's systems without going through the PM agent first — scope discipline is a deliberate part of this project, not busywork.
- Prioritize working, demoable increments over completeness. Each phase should end with something that runs.

## Tech stack

- **Language:** C++
- **Rendering:** Raylib
- **Physics:** TBD per phase (evaluate Bullet/Jolt for rigid-body, or a flight-dynamics-specific approach for the aircraft model itself)
- **Repo:** tac-flight-sim

## Roadmap / phases

1. Raylib fundamentals — throwaway scene, get comfortable with camera, model loading, basic lighting before touching real systems
2. MVP loop — one aircraft, empty sky, keyboard/joystick input, working camera
3. Core flight physics and controls — 6DOF, lift/drag/thrust, control surfaces
4. HUD and instrumentation — altitude, airspeed, heading, artificial horizon
5. RADAR / sensor detection — detection volume, target tracking, contact list
6. Missile lock-on and guidance — lock logic, proportional navigation
7. UAV / autonomous control — AI-controlled aircraft on the same flight model
8. Polish and portfolio packaging — README, architecture diagram, demo video/GIF

Current phase: **1 — Raylib fundamentals**

*(Update this line as phases complete — it's the fastest way for agents to know where things stand.)*

## Known strengths / gaps

The developer is confident in systems and physics programming (6DOF, simulation logic) and is deliberately investing extra time in the rendering/graphics side, which is a weaker area. Agents should account for this: don't assume graphics fluency, don't undersell the systems-programming judgment.

## Team roles

Available subagents in `.claude/agents/` — invoke by name or let Claude Code route automatically based on the request:

- **pm** — turns roadmap items into scoped tickets, keeps work inside the current phase, pushes back on scope creep. Consult before starting a new chunk of work.
- **architect** — reviews design and module boundaries *before* code is written. Consult before starting a new system (not for small changes within an existing one).
- **rendering-mentor** — teaches Raylib/graphics concepts, reviews rendering code for both correctness and idiomatic usage. Consult while working through phases 1, 2, and 4 especially.

Planned for later phases (not yet added):
- **code-reviewer** — post-implementation review of diffs (add once phase 3 starts)
- **qa** — independent test-writing and verification, especially for physics/RADAR/guidance correctness (add once phase 3 starts)
- **aerospace-consultant** — domain correctness for flight model, RADAR, and missile guidance (add before phase 5/6)
- **tech-writer** — README, architecture docs, demo packaging (add at phase 8)

## Workflow

1. Consult **pm** to get or confirm a scoped ticket for the current phase.
2. For a new system or major structural decision, consult **architect** before writing code.
3. Write the code yourself.
4. For rendering/graphics work, use **rendering-mentor** as needed while working, not just after.
5. (Later phases) Run through **code-reviewer** and **qa** once those are added.
