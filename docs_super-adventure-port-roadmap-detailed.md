# Super Adventure → Aurora Engine (C++)  
## Detailed 9-Phase Engineering Roadmap (No ImGui)

> Audience: engineer implementing the port  
> Style: senior/lead-directed tasks with clear scope, acceptance criteria, and deliberate implementation freedom.

---

## Working Rules (Applies to All Phases)

- [ ] Keep gameplay model code independent from engine/render code unless explicitly noted.
- [ ] Prefer small, reviewable commits per sub-task (not “whole phase in one commit”).
- [ ] Add instrumentation/assertions when behavior is ambiguous.
- [ ] Do not introduce broad framework abstractions unless required by a phase acceptance criterion.
- [ ] Track known debt in TODOs with owner + rationale (avoid vague “fix later”).

---

## Phase 1 — Engine Stabilization

### Intent
Stabilize lifecycle and ownership behavior so later systems are built on predictable engine semantics.

### Engineering Tasks
- [ ] Audit entity lifecycle transitions end-to-end (`Pending` → `Awake` → `Active` → `Destroyed`).
- [ ] Fix `Entity` constructor state assignment issues (including update order).
- [ ] Correct destroyed entity cleanup in `Layer` so container state matches runtime state.
- [ ] Verify `Inactive` entities are not updated/rendered; enforce via code, not comments/docs.
- [ ] Resolve `get_component<T>()` type conversion behavior with compile-time and runtime validation.

### Code Areas (Expected Touch Points)
- `src/entity.cpp`
- `src/layer.cpp`
- `public/core/entity.h`
- (optional) `public/core/layer.h` if cleanup APIs need tightening

### Implementation Guidance
- Treat this as correctness work, not feature work.
- Add lightweight diagnostics (assertions/log lines) around add/remove/update loops.
- Confirm behavior under:
  - entity marked destroyed mid-update
  - entity marked inactive then reactivated
  - component lookup on missing type

### Deliverables
- [ ] All identified bugs fixed.
- [ ] No behavior regression in current test/sample scene.
- [ ] Brief “engine invariants” note in code comments or docs.

### Out of Scope
- [ ] No new subsystems.
- [ ] No gameplay code.
- [ ] No API redesign unless directly required for bug fix.

---

## Phase 2 — Input System

### Intent
Provide deterministic per-frame input state usable by both game logic adapters and UI components.

### Engineering Tasks
- [ ] Introduce an engine-owned input state service (`InputHandler` or equivalent).
- [ ] Define frame lifecycle clearly: “begin frame, consume events, expose queried state, clear transient edges”.
- [ ] Support at minimum:
  - [ ] key held
  - [ ] key pressed this frame
  - [ ] key released this frame
  - [ ] mouse position
  - [ ] mouse left-click edge
- [ ] Route input access through engine/layer context without global static state leakage.
- [ ] Add one small component/helper that demonstrates input consumption from entity context.

### Code Areas
- New input service files
- `src/engine.cpp` input loop integration
- layer access plumbing
- optional input helper component

### Implementation Guidance
- Keep API simple; defer bind/action mapping.
- Make transient edges frame-stable (don’t let multiple polls clear state unexpectedly).
- Decide and document whether input is sampled before or after update tick.

### Deliverables
- [ ] A sample entity or layer can react to key press and mouse click reliably.
- [ ] Input state does not persist incorrectly across frames.
- [ ] Clear docs/comments for expected consumption semantics.

### Out of Scope
- [ ] Rebinding UI
- [ ] gamepads/controllers
- [ ] analog axis abstraction

---

## Phase 3 — Text & Font Rendering

### Intent
Move from debug text to production-capable text rendering usable by UI systems.

### Engineering Tasks
- [ ] Add and link `SDL3_ttf` into engine build/dependency graph.
- [ ] Implement a font lifecycle owner (load/cache/release).
- [ ] Implement a text render component or utility path that:
  - [ ] renders string with font+size+color
  - [ ] positions via transform/screen-space convention
  - [ ] handles failure states gracefully (missing font, invalid glyphs)
- [ ] Define ownership model for rendered text textures (per-frame temp vs cached).

### Code Areas
- build/dependency files
- font manager files
- text rendering component files

### Implementation Guidance
- Favor correctness and clarity over optimization initially.
- Decide early whether text is world-space or UI-space and keep that consistent.
- Add minimum logging around missing asset/font cases.

### Deliverables
- [ ] Labels can be rendered in-game with chosen fonts.
- [ ] No resource leaks when toggling labels or changing scenes/layers.
- [ ] Text rendering failure paths are visible and debuggable.

### Out of Scope
- [ ] Rich text styling
- [ ] advanced layout/wrapping engine
- [ ] localization pipeline

---

## Phase 4 — Minimal UI Component Set

### Intent
Implement only the UI surface needed to fully reproduce Super Adventure’s one-screen UX.

### Engineering Tasks
- [ ] Implement `Label` primitive (text + placement).
- [ ] Implement `Panel` primitive (background + optional border).
- [ ] Implement `Button` primitive with click callback and minimal state.
- [ ] Implement `ScrollableLog` with bounded history and append API.
- [ ] Implement `ListView` with two-column row rendering for inventory/quests.
- [ ] Define one consistent coordinate/layout scheme for all UI primitives.

### Code Areas
- new UI component files
- possible helper utilities for clipping/layout math

### Implementation Guidance
- Keep each widget intentionally shallow.
- Build simple API signatures that can survive to later phases.
- Decide whether clipping is hard (scissor) or soft (skip draw outside bounds); document tradeoff.
- Avoid building a general UI framework; ship the minimum viable set.

### Deliverables
- [ ] All required screen regions can be drawn and updated.
- [ ] Button callbacks are reliable and idempotent per click.
- [ ] Log and list components handle variable-length content predictably.

### Out of Scope
- [ ] dropdowns
- [ ] tooltip/focus manager
- [ ] keyboard navigation framework

---

## Phase 5 — Game Data Layer (Engine-Agnostic)

### Intent
Port all Super Adventure domain content into plain C++ data structures independent from Aurora runtime.

### Engineering Tasks
- [ ] Define domain models (`Item`, `Weapon`, `Potion`, `Monster`, `Quest`, `Location`, etc.).
- [ ] Define player-state models (`Player`, inventory entry, quest progress entry).
- [ ] Build world registry/lookup service (`GameWorld`) with robust ID lookup semantics.
- [ ] Port all world content faithfully from source project.
- [ ] Resolve known source inconsistencies intentionally (document every change from original behavior/data).

### Code Areas
- new `game/superadventure/data/` module
- RNG helper utility

### Implementation Guidance
- Keep all IDs explicit and stable.
- Prefer immutable world definition after `populate()` completion.
- Decide whether lookups return pointer/optional/reference wrappers; be consistent.
- Add lightweight data validation pass (duplicate IDs, missing references, invalid links).

### Deliverables
- [ ] World builds successfully from code/data.
- [ ] Validation pass reports zero unresolved references.
- [ ] Data module can be tested without engine initialization.

### Out of Scope
- [ ] rendering concerns
- [ ] input concerns
- [ ] direct UI update code

---

## Phase 6 — Gameplay Rules / Controller Layer

### Intent
Implement game behavior as pure logic orchestrating player + world state transitions.

### Engineering Tasks
- [ ] Implement movement/action entry points (`move_to`, `use_weapon`, `use_potion`).
- [ ] Implement gating checks (level, required item).
- [ ] Implement quest acquisition/completion and inventory reward/consumption rules.
- [ ] Implement combat resolution loop and death/respawn behavior.
- [ ] Implement message/event callback surface for UI consumption.
- [ ] Add deterministic seams (injectable RNG optional but recommended).

### Code Areas
- `game/superadventure/game_controller.*`
- related data model mutation helpers

### Implementation Guidance
- Keep side effects explicit and centralized.
- Avoid hidden mutations spread across unrelated model classes.
- Build in-phase verification scenarios (“given state X, action Y produces Z messages + Z’ state”).
- Preserve original game behavior first, then tune later.

### Deliverables
- [ ] All player actions execute through controller API.
- [ ] UI can subscribe to textual events without logic coupling.
- [ ] Rule behavior matches expected baseline scenarios.

### Out of Scope
- [ ] direct SDL/UI calls
- [ ] advanced balancing changes
- [ ] NPC objectization

---

## Phase 7 — AdventureLayer Integration (Playable Port)

### Intent
Compose data + controller + UI primitives into a full playable one-screen game.

### Engineering Tasks
- [ ] Create `AdventureLayer` that owns/wires world, player, controller, and UI entities.
- [ ] Build the complete screen composition (stats, location, log, inventory, quests, controls).
- [ ] Implement per-frame UI sync from game state.
- [ ] Implement control visibility/state logic based on gameplay state.
- [ ] Ensure callback flow (controller → log/UI) is robust across scene lifetime.

### Code Areas
- `game/superadventure/adventure_layer.*`
- initialization/bootstrap hookup in sample entrypoint

### Implementation Guidance
- Keep UI refresh deterministic; avoid one-off stale fields.
- Introduce small adapter helpers for repeated “state → UI row” mapping.
- Guard against null/empty content paths in first frame before world/player fully initialized.

### Deliverables
- [ ] End-to-end game loop playable in-engine.
- [ ] All core actions reachable through UI.
- [ ] No hard crashes during prolonged play sessions.

### Out of Scope
- [ ] multi-screen architecture
- [ ] transition animation
- [ ] fancy UX polish

---

## Phase 8 — Save/Load Persistence

### Intent
Persist meaningful player progress and restore it safely.

### Engineering Tasks
- [ ] Add JSON serialization dependency and integration.
- [ ] Define save schema (explicit keys, stable IDs, optional version field).
- [ ] Implement `save_player` and `load_player` with validation and fallback behavior.
- [ ] Integrate save-on-quit and load-on-start lifecycle points.
- [ ] Handle corrupted/missing save gracefully.

### Code Areas
- persistence files under `game/superadventure/`
- layer bootstrap/quit hooks

### Implementation Guidance
- Prefer explicit schema over “serialize everything blindly”.
- Validate referential integrity during load (unknown item/quest/location IDs).
- Add migration placeholder strategy even if only one version exists now.

### Deliverables
- [ ] Save file reliably written and reloaded.
- [ ] Invalid save does not crash runtime; fallback path works.
- [ ] Player state continuity validated across restarts.

### Out of Scope
- [ ] multi-slot saves
- [ ] cloud sync
- [ ] world-state persistence beyond player model

---

## Phase 9A — Sprite/Texture Foundation

### Intent
Enable textured rendering pipeline needed for visual upgrades.

### Engineering Tasks
- [ ] Implement texture cache/manager with clear ownership and teardown.
- [ ] Implement static sprite rendering component.
- [ ] Implement frame-based animated sprite component.
- [ ] Define source-rect and tint handling conventions.
- [ ] Validate render ordering with existing UI/component stack.

### Code Areas
- texture manager files
- sprite/animated sprite components

### Implementation Guidance
- Keep animation controller simple (looping timeline).
- Avoid overbuilding blend modes/shader abstraction now.
- Log missing texture failures with useful context.

### Deliverables
- [ ] Static sprites render reliably.
- [ ] Animations tick correctly over time.
- [ ] Resource cleanup verified on shutdown.

### Out of Scope
- [ ] full animation state machine
- [ ] material/shader graph
- [ ] dynamic atlasing

---

## Phase 9B — Combat Enemy Sprite Presentation

### Intent
Overlay visual monster representation in combat without rewriting game rules.

### Engineering Tasks
- [ ] Add combat viewport rendering region to `AdventureLayer`.
- [ ] Map monster IDs to sprite assets.
- [ ] Drive sprite state from combat lifecycle events.
- [ ] Add minimal hit/death feedback effects (timed tint/visibility changes).

### Implementation Guidance
- Keep controller/UI contract unchanged where possible.
- Use additive callbacks if additional combat visual events are needed.
- Ensure visual state resets cleanly after each encounter.

### Deliverables
- [ ] Encountering any monster displays expected sprite.
- [ ] Combat visuals update correctly on hit/death/respawn.

### Out of Scope
- [ ] cinematic combat animations
- [ ] particle/VFX system

---

## Phase 9C — Inventory Visual Upgrade

### Intent
Replace text-row inventory display with slot/icon-based UX while preserving data contracts.

### Engineering Tasks
- [ ] Implement inventory slot component (icon + quantity badge).
- [ ] Implement inventory panel/grid component.
- [ ] Implement shared tooltip surface for hover details.
- [ ] Replace list-view wiring in `AdventureLayer` inventory panel.

### Implementation Guidance
- Keep quest list text-based for now.
- Centralize item ID → icon path resolution.
- Decide behavior for overflow when inventory exceeds visible slots.

### Deliverables
- [ ] Inventory displays as icon grid.
- [ ] Quantity and tooltip details are accurate.
- [ ] Existing gameplay actions continue to function unchanged.

### Out of Scope
- [ ] drag/drop
- [ ] sort/filter
- [ ] multi-panel equipment comparison

---

## Phase 9D — Location Backgrounds (Stretch)

### Intent
Add low-risk visual atmosphere with per-location background art.

### Engineering Tasks
- [ ] Add a background render entity behind UI.
- [ ] Map locations (or location groups) to background assets.
- [ ] Update background on location transitions.

### Implementation Guidance
- Treat this as cosmetic and low coupling.
- Build safe fallback when no background exists.

### Deliverables
- [ ] Background changes correctly with location.
- [ ] No impact on game logic/state behavior.

### Out of Scope
- [ ] parallax
- [ ] transition effects
- [ ] day/night cycles

---

## Master Tracking Checklist (All Phases)

- [ ] Phase 1 complete
- [ ] Phase 2 complete
- [ ] Phase 3 complete
- [ ] Phase 4 complete
- [ ] Phase 5 complete
- [ ] Phase 6 complete
- [ ] Phase 7 complete
- [ ] Phase 8 complete
- [ ] Phase 9A complete
- [ ] Phase 9B complete
- [ ] Phase 9C complete
- [ ] Phase 9D complete (optional/stretch)