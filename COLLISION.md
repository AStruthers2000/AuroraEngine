# Collision System Design

## Overview

AABB ↔ AABB collision for the Aurora Engine. Detection is a global (Layer-level)
responsibility; response is a local (Component/Entity) responsibility. This split is
necessary because detecting overlaps requires comparing pairs of entities — no single
`CollisionComponent` has visibility into all other entities on the Layer.

---

## Execution Order

Collision fits into the engine's frame as a fixed-rate step, after entities have integrated
their velocities but before entities react to results:

```
fixed_update_layer(fixed_dt):
    fixed_update(fixed_dt)              // Layer hook: set up physics frame if needed
    fixed_update_entities(fixed_dt)     // Movement components integrate velocity → intended positions
    collision_subsystem.resolve(...)    // Detect overlaps, correct positions, fire callbacks

update_layer(delta_time):
    update(delta_time)                  // Layer hook: game rules, spawning
    update_entities(delta_time):
        per entity:
            update_components(dt)       // comp::update() — input, pre-physics logic
            update(dt)                  // entity::update() — game decisions
            late_update_components(dt)  // comp::late_update() — camera, post-physics reads
            late_update(dt)             // entity::late_update() — react to collision results
    late_update(delta_time)             // Layer hook: win conditions, score
```

Collision runs in `fixed_update_layer` because velocity integration + AABB resolution is
physics — it needs a deterministic, fixed-rate step. Running it in the variable update risks
fast-moving objects tunnelling through thin colliders during a frame hitch.

The entity's `late_update()` (last step in `update_layer`) is the natural place to react to
collision results set during the fixed pass (e.g. play a hit sound, apply damage, trigger
an animation). The one-frame delay between the fixed step and the variable reaction is
invisible at 60 Hz.

---

## Responsibility Split

| What | Who owns it |
|---|---|
| AABB shape (position + size) | `CollisionComponent`, derived from sibling `TransformComponent` |
| `is_dynamic` / `is_static` / `is_trigger` flags | `CollisionComponent` |
| Collision response callback registration | `CollisionComponent` (registered during `awake()`) |
| Iterating all pairs and detecting overlaps | `CollisionSubsystem` (called by `Layer`) |
| Position correction (penetration push-back) | `CollisionSubsystem` applies it directly, or fires callback so `CollisionComponent` applies it |
| Game reaction (damage, bounce, sound, animation) | Entity `late_update()`, or the registered callback on `CollisionComponent` |

---

## Component Design (`CollisionComponent`)

```cpp
class CollisionComponent : public Component
{
public:
    enum class ECollisionType { Dynamic, Static, Trigger };

    using CollisionCallback = std::function<void(CollisionComponent& other, glm::vec2 push_back)>;

    void register_collision_response(CollisionCallback callback);
    void fire_collision_response(CollisionComponent& other, glm::vec2 push_back);

    SDL_FRect get_aabb() const;   // derived from sibling TransformComponent
    ECollisionType get_type() const;
    bool is_dynamic() const { return m_type == ECollisionType::Dynamic; }

protected:
    virtual void awake() override;  // gets sibling TransformComponent, registers with subsystem

private:
    ECollisionType m_type{ ECollisionType::Dynamic };
    std::weak_ptr<TransformComponent> m_transform{};
    CollisionCallback m_response{};
};
```

---

## Subsystem Design (`CollisionSubsystem`)

Lives on each `Layer`. Each Layer has its own subsystem — cross-layer collision is an
explicit opt-in (not needed for the initial implementation).

```cpp
class CollisionSubsystem
{
public:
    void register_collider(std::weak_ptr<CollisionComponent> component);
    void unregister_collider(std::weak_ptr<CollisionComponent> component);

    // Called from Layer::fixed_update_layer(), after fixed_update_entities()
    void resolve();

private:
    // Returns the overlap vector (zero if no overlap)
    static glm::vec2 compute_overlap(SDL_FRect const& a, SDL_FRect const& b);

    std::vector<std::weak_ptr<CollisionComponent>> m_colliders;
};
```

`resolve()` iterates all pairs (O(n²) — acceptable for small entity counts; upgrade to a
spatial grid or sweep-and-prune later if needed), computes penetration, and for each
overlapping pair:

1. Corrects positions by pushing dynamic bodies out of static/other-dynamic bodies.
2. Calls `fire_collision_response()` on each involved `CollisionComponent`.

---

## Layer Integration

```cpp
// In Layer header:
CollisionSubsystem m_collision_subsystem;

// In Layer::fixed_update_layer():
void Layer::fixed_update_layer(float fixed_dt)
{
    if (m_paused) return;
    fixed_update(fixed_dt);
    fixed_update_entities(fixed_dt);
    m_collision_subsystem.resolve();  // NEW: runs after velocity integration
}
```

`CollisionComponent::awake()` calls:
```cpp
get_owning_layer().get_collision_subsystem().register_collider(shared_from_this());
```

A `get_collision_subsystem()` accessor is added to `Layer`.

---

## Collision Types

| Type | Behaviour |
|---|---|
| `Static` | Never moves; other dynamic bodies are pushed out of it |
| `Dynamic` | Can be pushed by resolution; two dynamics split the push-back |
| `Trigger` | Detects overlap but applies no position correction; fires callback only |

---

## Penetration Resolution (MTV — Minimum Translation Vector)

For two AABBs A and B:

```
overlap_x = min(A.right, B.right) - max(A.left, B.left)
overlap_y = min(A.bottom, B.bottom) - max(A.top, B.top)

if overlap_x < overlap_y:
    push_back = { ±overlap_x, 0 }   // resolve on the shallower axis
else:
    push_back = { 0, ±overlap_y }
```

The sign is determined by the relative position of the two centers. Dynamic vs. Static: the
dynamic body absorbs the full push-back. Dynamic vs. Dynamic: each absorbs half.

---

## Future Considerations

- **Spatial partitioning**: replace the O(n²) pair loop with a uniform grid or
  sweep-and-prune once entity counts exceed ~50 collidables per layer.
- **Collision layers/masks**: bitmask on `CollisionComponent` to filter which types of
  objects can collide with each other (e.g. player bullets don't collide with player).
- **Cross-layer collision**: `Engine` could maintain a global `CollisionSubsystem` for
  entities that opt in with a flag.
- **Continuous collision detection (CCD)**: swept AABB test for fast-moving objects to
  prevent tunnelling at variable update rates.
