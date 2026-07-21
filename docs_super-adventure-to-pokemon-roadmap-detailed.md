# Super Adventure → Pokémon-Style Refactor/Extension  
## Detailed Engineering Roadmap (Post 9-Phase Port)

> Audience: engineer(s) evolving the project into a Gen 1–5 style monster RPG.  
> Assumption: Super Adventure 9-phase plan is complete and stable.

---

## Transformation Constraints

- [ ] Preserve engine/game separation where possible.
- [ ] Keep battle rules testable without renderer.
- [ ] Keep map/encounter data externalizable.
- [ ] Build for content scalability (species/moves/maps/trainer data volume).

---

## Phase P1 — Tilemap + Warp Infrastructure

### Intent
Replace node-based world traversal with map-based spatial traversal and warps.

### Engineering Tasks
- [ ] Define tilemap data model:
  - [ ] dimensions
  - [ ] tile layers
  - [ ] collision layer
  - [ ] metadata tags
- [ ] Implement map loader from external format (prefer JSON/Tiled export or equivalent).
- [ ] Implement tile renderer supporting layer ordering (ground/objects/overdraw).
- [ ] Implement warp zone model (source trigger + destination map/tile).
- [ ] Implement map transition service that can unload/load map context and reposition player.

### Code Areas
- `game/pokemon/world/tilemap/*`
- `TileMapComponent` (render integration)
- map transition coordinator in overworld layer

### Implementation Guidance
- Keep collision and rendering concerns separate (don’t infer collision from art layer).
- Use integer tile coordinates for simulation, float only for render interpolation if needed.
- Add validation tooling for map references (invalid warp targets, missing map IDs).

### Deliverables
- [ ] Player can move on map tiles and trigger warps reliably.
- [ ] Layered map rendering works with deterministic draw order.

### Out of Scope
- [ ] editor tooling
- [ ] animated tiles
- [ ] procedural maps

---

## Phase P2 — Camera and World-Space Rendering Conventions

### Intent
Establish a consistent camera model for scrolling overworld scenes.

### Engineering Tasks
- [ ] Implement camera entity/service with:
  - [ ] target follow
  - [ ] map bounds clamp
  - [ ] viewport dimension awareness
- [ ] Standardize world-space vs screen-space transforms across components.
- [ ] Ensure UI render path remains screen-space while map/entities remain world-space.
- [ ] Validate camera transitions on warp/map-change events.

### Code Areas
- camera module
- render components consuming camera offset
- layer render orchestration

### Implementation Guidance
- Document transform conventions in one source-of-truth comment/doc.
- Avoid ad hoc per-component offset hacks.
- Defer zoom/lerp complexity unless required.

### Deliverables
- [ ] Smooth and bounded camera follow behavior.
- [ ] No coordinate-space confusion between HUD and world entities.

### Out of Scope
- [ ] multiple cameras
- [ ] cinematic rails
- [ ] dynamic zoom behavior

---

## Phase P3 — Overworld Player Movement Model

### Intent
Implement Pokémon-style tile-locked walking and directional animation.

### Engineering Tasks
- [ ] Build tile-step movement controller:
  - [ ] one-tile movement commitments
  - [ ] movement blocking via collision map
  - [ ] input buffering during step transitions
- [ ] Implement direction/facing state (N/E/S/W).
- [ ] Integrate movement with animated sprite direction sets.
- [ ] Emit tile-arrival events for downstream systems (encounters, triggers, scripts).

### Code Areas
- player movement component/module
- directional animation mapping
- trigger/event hook points

### Implementation Guidance
- Keep gameplay simulation in tile coordinates.
- Distinguish simulation “arrival” from render interpolation.
- Prevent diagonal movement unless intentionally added.

### Deliverables
- [ ] Movement feels deterministic and responsive.
- [ ] Tile arrival events fire exactly once per completed step.

### Out of Scope
- [ ] run/bike/surf
- [ ] analog free movement
- [ ] movement abilities requiring map script integration

---

## Phase P4 — NPC Presence + Dialogue Flow

### Intent
Introduce interactable NPCs with dialogue and map presence constraints.

### Engineering Tasks
- [ ] Implement NPC data/component model:
  - [ ] position/facing
  - [ ] optional behavior state
  - [ ] dialogue reference
- [ ] Implement interaction query on player-facing tile.
- [ ] Build dialogue presentation surface (paged text, continue/close flow).
- [ ] Add NPC occupancy into collision/path blocking logic.
- [ ] Add minimal scripting hooks for “on-talk” events.

### Code Areas
- NPC systems
- dialogue UI layer/component
- overworld interaction handlers

### Implementation Guidance
- Keep dialogue content data-driven and externalizable.
- Ensure dialogue flow blocks movement/input appropriately while active.
- Keep first pass linear (no branching tree editor needed yet).

### Deliverables
- [ ] Player can interact with NPCs and advance dialogue pages.
- [ ] NPCs correctly block movement and preserve facing/state.

### Out of Scope
- [ ] cinematic cutscene framework
- [ ] branching dialogue tools
- [ ] quest journal integration

---

## Phase P5 — Battle System Core (Pokémon Ruleset Baseline)

### Intent
Create robust turn-based monster combat separate from rendering concerns.

### Engineering Tasks
- [ ] Define battle domain models:
  - [ ] species
  - [ ] move
  - [ ] instantiated monster
  - [ ] party
  - [ ] type chart
- [ ] Implement battle state machine:
  - [ ] pre-turn selection
  - [ ] turn resolution order
  - [ ] move execution
  - [ ] status effect ticks
  - [ ] faint resolution
  - [ ] battle end conditions
- [ ] Implement damage/effectiveness formulas and critical hooks.
- [ ] Emit battle events/messages for UI consumption.
- [ ] Build battle presentation layer using existing UI/sprite infrastructure.

### Code Areas
- `game/pokemon/battle/*` (engine-agnostic core + layer adapters)
- battle layer/menu components

### Implementation Guidance
- Keep engine-agnostic battle core pure and deterministic where possible.
- Separate random generation seams for reproducibility/testing.
- Define explicit turn phases to avoid “spaghetti resolution logic”.

### Deliverables
- [ ] Full battle lifecycle works for wild encounter baseline.
- [ ] Battle UI reflects state transitions accurately.

### Out of Scope
- [ ] double/triple battles
- [ ] ability system (if deferred)
- [ ] advanced AI optimization

---

## Phase P6 — Wild Encounter Pipeline

### Intent
Attach exploration to battle entry through encounter zones/tables.

### Engineering Tasks
- [ ] Define per-map encounter table format with weighted species/level ranges.
- [ ] Tag encounter-capable tiles/zones in map metadata.
- [ ] Implement step-triggered encounter checks with configurable rates.
- [ ] Generate encounter instance and transition to battle context.
- [ ] Handle post-battle return-to-overworld placement/state.

### Code Areas
- encounter model + generator
- overworld tile-arrival consumers
- battle transition glue code

### Implementation Guidance
- Keep encounter RNG independent from battle RNG if practical.
- Add telemetry/logging for encounter rates during balancing.
- Support map-specific tuning without code changes.

### Deliverables
- [ ] Encounter triggers behave as expected by zone.
- [ ] Encounter distribution aligns with configured weights.

### Out of Scope
- [ ] repel/fishing/surf mechanics
- [ ] roaming legendary behavior
- [ ] weather/time-based encounter modifiers

---

## Phase P7 — Trainer Battles + Shop Economy

### Intent
Add core non-wild gameplay loops: trainer combat and item economy.

### Engineering Tasks
- [ ] Extend NPC model to trainer NPCs:
  - [ ] party roster
  - [ ] pre/post dialogue
  - [ ] defeated persistence
- [ ] Implement trainer battle trigger mode(s):
  - [ ] interaction-triggered minimum
  - [ ] optional line-of-sight challenge
- [ ] Integrate trainer reward logic (money payout, defeat state).
- [ ] Implement shop NPC and shop UI flow:
  - [ ] stock lists
  - [ ] buy/sell transactions
  - [ ] quantity handling
  - [ ] money validation

### Code Areas
- trainer systems
- shop systems/layers
- persistence extension for trainer-defeated flags

### Implementation Guidance
- Start with deterministic trainer trigger approach, then add LOS complexity.
- Keep economy model explicit; avoid hidden side effects during transactions.
- Persist trainer defeat states to prevent repeat battles unless intentionally allowed.

### Deliverables
- [ ] Trainer encounter and post-defeat behavior works.
- [ ] Shop loop (buy/sell) functions with proper validation.

### Out of Scope
- [ ] dynamic inventories
- [ ] rematch schedules
- [ ] player-to-player trade economy

---

## Phase P8 — Full Menu Stack (Field UX)

### Intent
Implement the expected menu-driven UX for party, bag, dex, and save flows.

### Engineering Tasks
- [ ] Implement start menu layer and navigation control flow.
- [ ] Implement party screen with status/HP summary and selection semantics.
- [ ] Implement bag screen with item pockets/categories.
- [ ] Implement Pokédex screen with seen/owned state views.
- [ ] Implement save confirmation flow and user feedback.
- [ ] Integrate Pokémon center healing interaction flow.

### Code Areas
- menu layers under `game/pokemon/ui/*`
- save/inventory/pokedex adapters
- shared menu navigation helpers

### Implementation Guidance
- Keep menu input handling consistent across all layers.
- Reuse UI primitives but avoid forcing one component to solve all menu types.
- Define a clear “modal layer” pattern (input capture rules).

### Deliverables
- [ ] All core field menus open/close/navigate correctly.
- [ ] Menu actions mutate correct game state and persist as expected.

### Out of Scope
- [ ] PC box full management (can be staged)
- [ ] advanced filters/sorting
- [ ] cosmetic transitions/animations

---

## Phase P9 — Progression Loop Completion (Capture/Level/Evolution)

### Intent
Complete the signature loop: catch, train, level, evolve, learn moves.

### Engineering Tasks
- [ ] Implement XP gain and level-up recalculation pipeline.
- [ ] Implement move learnset progression with “replace move” flow.
- [ ] Implement evolution trigger checks and evolution transition flow.
- [ ] Implement capture mechanics and party/storage placement logic.
- [ ] Persist newly required progression state (owned/seen, evolution history if needed).

### Code Areas
- progression systems in battle/domain modules
- UI overlays for learn/evolution/capture outcomes
- save schema extensions

### Implementation Guidance
- Separate progression resolution from rendering prompts.
- Stage complexity:
  - initial simplified progression
  - then edge-case handling (full party, duplicate learnset conflicts, etc.)
- Add regression scenarios for level-up and move replacement behavior.

### Deliverables
- [ ] End-to-end progression loop works in normal gameplay.
- [ ] Captured entities persist and appear in party/storage as expected.

### Out of Scope
- [ ] breeding/daycare
- [ ] advanced post-Gen5 mechanics
- [ ] networked trading

---

## Phase P10 — Audio Integration & Presentation Polish

### Intent
Add BGM/SFX systems to support game feel and state transitions.

### Engineering Tasks
- [ ] Add audio dependency (`SDL3_mixer` or chosen equivalent).
- [ ] Implement audio manager lifecycle + channel policy.
- [ ] Wire BGM routing by scene/context:
  - [ ] map BGM
  - [ ] battle BGM
  - [ ] transition/recovery behavior
- [ ] Add key SFX hooks:
  - [ ] menu confirm/cancel
  - [ ] attack hit/miss
  - [ ] capture shakes/success/fail
  - [ ] level/evolution stingers
- [ ] Implement fallback behavior for missing audio assets.

### Code Areas
- audio manager module
- layer transition hooks
- battle/menu event handlers

### Implementation Guidance
- Prioritize deterministic playback control over advanced mixing features.
- Keep volume categories configurable (master/bgm/sfx).
- Build dead-simple asset mapping first, then refactor if needed.

### Deliverables
- [ ] Audio behaves consistently across overworld/battle/menu contexts.
- [ ] Missing asset failures are non-fatal and diagnosable.

### Out of Scope
- [ ] positional/spatial audio
- [ ] dynamic adaptive soundtrack
- [ ] advanced middleware integration

---

## Cross-Cutting Backlog (Run in Parallel Where Appropriate)

- [ ] Data externalization strategy (species/moves/maps/trainers/items)
- [ ] Content validation tooling (ID integrity, orphan references, bad warps)
- [ ] Save schema versioning/migration policy
- [ ] Deterministic test scenarios for battle and progression
- [ ] Performance checks for tile rendering and battle UI redraws
- [ ] Error-handling standardization for missing assets/data

---

## Master Tracking Checklist

- [ ] P1 Tilemap + warps complete
- [ ] P2 Camera complete
- [ ] P3 Overworld movement complete
- [ ] P4 NPC/dialogue complete
- [ ] P5 Battle core complete
- [ ] P6 Wild encounters complete
- [ ] P7 Trainer + shop complete
- [ ] P8 Menu stack complete
- [ ] P9 Progression complete
- [ ] P10 Audio complete