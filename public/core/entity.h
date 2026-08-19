////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Base Entity class. Root of all objects in a Layer.
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CORE_ENTITY_H
#define CORE_ENTITY_H

#include "core/component.h"
#include "core/layer.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Core
{

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Base Entity class. Root of all objects in a Layer.
///
/// Entity is the fundamental game object. Every visible or behavioral thing in a game - a player
/// character, a bullet, a UI element - is an Entity. Entities live on a Layer, which in turn is
/// owned by the Engine.
///
/// To create a game object, derive from Entity and override the virtual lifecycle hooks
/// (on_awake, on_start, on_update, on_render, etc.). Attach Components in the constructor or
/// during on_awake() using add_component<T>(). Access Components via get_component<T>() - cache the
/// returned @c std::weak_ptr during on_awake() or on_start() rather than calling it every frame.
////////////////////////////////////////////////////////////////////////////////////////////////////
class Entity
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Types
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Lifecycle state of the Entity. Controls how it participates in the main loop.
    ///
    /// @note Active <-> Inactive is freely reversible via set_active() / set_inactive().
    ///       All other transitions are one-way. Destroyed is terminal.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    enum class EState
    {
        Active,     ///< Normal operating state. update_entity() is called every frame.
        Inactive,   ///< Paused. update_entity() is skipped, but render_entity() still runs.
                    ///< Transition freely to/from Active via set_active() / set_inactive().
        Disabled,   ///< Effectively cut out of the main loop, but not deallocated. Doesn't receive
                    ///< any events, updates, or rendering passes.
        Pending,    ///< Constructed but not yet initialized. The next initialization pass will
                    ///< call awake_entity(), transitioning this Entity to Awoken.
        Awoken,     ///< Awake phase complete. All Components are initialized and accessible.
                    ///< start_entity() will be called once all sibling Entities in the same
                    ///< initialization pass have also reached this state.
        Destroyed,  ///< Marked for destruction. The Entity will be cleaned up and removed by the
                    ///< owning Layer at the end of the current frame. Terminal state.
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Wraps either a Layer or a parent Entity as the owner of a new Entity. Both convert
    ///        implicitly, so derived entity constructors need only a single overload.
    ///
    /// @note  Implicit conversion example:
    ///        @code
    ///        // Adding a root Entity to a Layer:
    ///        add_entity<MyEntity>();          // Layer implicitly converts to Owner
    ///
    ///        // Adding a child Entity from inside an Entity:
    ///        add_child_entity<MyChild>();     // *this implicitly converts to Owner
    ///        @endcode
    ////////////////////////////////////////////////////////////////////////////////////////////////
    struct Owner
    {
        Owner(Layer&  layer)  noexcept : m_layer(&layer),                  m_parent(nullptr)  {}
        Owner(Entity& entity) noexcept : m_layer(&entity.get_owning_layer()), m_parent(&entity) {}

        Layer&  layer()  const { return *m_layer;  }
        Entity* parent() const { return m_parent; }

    private:
        Layer*  m_layer;
        Entity* m_parent;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Construction parameters for Entity and all derived classes.
    ///
    /// Derived classes should inherit from Entity::Configuration and add their own fields:
    /// @code
    /// struct Configuration : Entity::Configuration
    /// {
    ///     glm::vec2 position{ 0.f, 0.f };
    /// };
    /// @endcode
    ////////////////////////////////////////////////////////////////////////////////////////////////
    struct Configuration
    {
        /// @brief Controls the order in which this Entity is updated relative to siblings on the
        ///        same Layer. Lower values update first. Defaults to DEFAULT_SORTING_ORDER.
        std::uint8_t update_order = DEFAULT_SORTING_ORDER;
    };


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Construction & Destruction
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Entity constructor. Accepts either a Layer or a parent Entity as owner via the
    ///        implicitly-converting Owner wrapper.
    ///
    /// @note  Add Components here via add_component<T>() or in on_awake(). Components added
    ///        during construction are guaranteed to be accessible from on_awake() onward.
    ///
    /// @param [in] owner  - The Layer or parent Entity that will own this Entity.
    /// @param [in] config - Optional construction parameters.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    Entity(Owner owner, Configuration const& config = {});

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Entity destructor. Virtual to support polymorphic deletion.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~Entity() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Virtual Lifecycle Hooks
    //
    // Override these in a derived class to implement custom Entity behavior. The engine calls them
    // through the non-virtual orchestrators (awake_entity, update_entity, etc.) - override these
    // hooks, not those orchestrators.
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Phase 1 initialization hook. Called once, before on_start(), after all pending
    ///        Components have been flushed to the active store.
    ///
    /// @note  Contract: all Components added during construction are accessible via
    ///        get_component<T>(). Cache your weak_ptr references to sibling Components here.
    ///
    /// @note  Must not: access Components or state on other Entities - their on_awake() is not
    ///        guaranteed to have run yet. Use on_start() for cross-Entity setup.
    ///
    /// @note  Use this override to:
    ///         - Add Components that depend on constructor arguments.
    ///         - Cache weak_ptr references to sibling Components for per-frame use.
    ///         - Perform any initialization that does not require other Entities to be ready.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void on_awake() {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Phase 2 initialization hook. Called once, after all Entities in the same
    ///        initialization pass have completed on_awake().
    ///
    /// @note  Contract: all Entities that were pending in the same batch have completed on_awake().
    ///        It is safe to access other Entities and their Components.
    ///
    /// @note  Use this override to:
    ///         - Set up references or state that depend on other Entities having been initialized.
    ///         - Register this Entity with a game-mode manager or other cross-Entity system.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void on_start() {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Per-frame update hook. Called every frame while the Entity is Active.
    ///
    /// @note  Contract: all Components on this Entity are active and accessible. Other Entities
    ///        that were active at the start of this frame are also safe to access.
    ///
    /// @note  Use this override to:
    ///         - Handle input-driven state changes.
    ///         - Apply per-frame logic (movement, animation state machines, etc.).
    ///
    /// @param [in] delta_time - Seconds elapsed since the last frame.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void on_update(float delta_time) {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Per-frame late-update hook. Called each frame after on_update() and after all
    ///        Component on_update() calls have completed. Component on_late_update() calls run
    ///        after this returns.
    ///
    /// @note  Use this override to:
    ///         - Read fully-resolved per-frame state produced by on_update() or Components
    ///           (e.g. finalize a position after physics has been applied).
    ///         - Feed resolved state to dependent systems via broadcast_event().
    ///
    /// @param [in] delta_time - Seconds elapsed since the last frame.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void on_late_update(float delta_time) {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Fixed-rate update hook. Called at a fixed timestep, independent of frame rate.
    ///        May be called zero or more times per rendered frame.
    ///
    /// @note  Use this override for:
    ///         - Physics and collision logic that must be frame-rate independent.
    ///         - Any simulation that requires a stable, predictable timestep.
    ///
    /// @param [in] fixed_dt - The fixed timestep in seconds (see Engine::set_fixed_timestep()).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void on_fixed_update(float fixed_dt) {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Per-frame render hook. Called every frame for Active and Inactive Entities.
    ///
    /// @note  Must not: mutate engine state. Do not call destroy_entity(), add_component(),
    ///        set_active(), or any other state-mutating method here. The render pass is a
    ///        read-only pass over Entity and Component state.
    ///
    /// @note  Use this override to:
    ///         - Draw custom geometry or UI elements using the provided renderer.
    ///         - Render debug overlays.
    ///
    /// @param [in] renderer - The SDL renderer for the current frame.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void on_render(SDL_Renderer* renderer) {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Cleanup hook. Called once before this Entity is destroyed.
    ///
    /// @note  Contract: all Components on this Entity are still alive and accessible. Component
    ///        on_cleanup() calls run after this returns.
    ///
    /// @note  Must not: call add_component<T>() or destroy_entity() here.
    ///
    /// @note  Use this override to:
    ///         - Release resources that require explicit teardown.
    ///         - Unregister from game-mode systems or event listeners.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void on_cleanup() {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Event hook. Called when an event is propagated down to this Entity from the owning
    ///        Layer. Components on this Entity receive the event first; this is called only if
    ///        the event was not already handled by a Component.
    ///
    /// @note  Use an EventDispatcher inside this override to route events to typed handler
    ///        lambdas. Return @c true from a dispatch handler to mark the event handled and stop
    ///        further propagation.
    ///
    /// @note  Use this override to:
    ///         - Respond to input events (key presses, mouse clicks).
    ///         - React to custom game events broadcast through the engine event system.
    ///
    /// @param [in] event - The event being propagated.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void on_event(Event& event) {}


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Lifecycle Management
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Marks this Entity for deferred destruction. The Entity will be cleaned up and
    ///        removed from its owning Layer at the end of the current frame.
    ///
    /// @note  This call is irreversible - once marked Destroyed, the Entity cannot return to any
    ///        other state. Contrast with set_inactive(), which is fully reversible.
    ///
    /// @note  Must not: be called from within on_render().
    ///
    /// @warning After calling destroy_entity(), do not access the Entity's state or Components.
    ///          Any weak_ptr held by other objects will return @c nullptr from .lock() once the
    ///          Entity has been removed.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void destroy_entity();


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // State & Control
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Transitions this Entity to the Active state, allowing update_entity() to run.
    ///
    /// @note  Has no effect if the Entity is in the Pending, Awoken, or Destroyed state.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void set_active();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Transitions this Entity to the Inactive state. While Inactive, update_entity() is
    ///        skipped but render_entity() continues to run. Fully reversible via set_active().
    ///
    /// @note  Has no effect if the Entity is in the Pending, Awoken, or Destroyed state.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void set_inactive();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Transitions this Entity to the Disabled state. While Disabled, all lifecycle hooks
    ///        are skipped; no update_entity(), no render_entity(), no events, etc.
    ///
    /// @note  Has no effect if the Entity is in the Pending, Awoken, or Destroyed state.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void set_disabled();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return The current EState of this Entity.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    EState get_entity_state() const { return m_state; }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return The update order of this Entity relative to siblings on the same Layer. Lower
    ///         values update first.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    std::uint8_t get_update_order() const { return m_update_order; }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return A reference to the Layer that owns this Entity.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    Layer& get_owning_layer() const { return m_owning_layer; }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return A raw pointer to the parent Entity, or @c nullptr if this is a root Entity.
    ///
    /// @note  Do not store this pointer long-term. Use has_parent() to guard before accessing it.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    Entity* get_parent() const { return m_parent; }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return @c true if this Entity has a parent Entity (i.e. was added via add_child_entity).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool has_parent() const { return m_parent != nullptr; }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Events
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Broadcasts an event engine-wide. The event propagates through all Layers and
    ///        Entities in reverse Layer stack order (top Layer receives it first). Any Entity or
    ///        Component can mark it handled to stop propagation early.
    ///
    /// @note  Safe to call from on_update(), on_event(), or on_fixed_update(). Not safe from
    ///        on_render().
    ///
    /// @param [in] event - The event to broadcast. The caller retains ownership.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void broadcast_event(Event& event);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Broadcasts an event within this Entity only. The event is passed to each attached
    ///        Component's on_event(), then to this Entity's own on_event(). No other Entities or
    ///        Layers receive the event.
    ///
    /// @note  Prefer this over broadcast_event() when the event only needs to be seen by
    ///        Components on this Entity - avoids the overhead of a full engine-wide dispatch.
    ///
    /// @param [in] event - The event to broadcast within this Entity.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void broadcast_event_within_entity(Event& event);


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Component & Child Entity Management
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Constructs a child Entity of type TEntity owned by this Entity. The child is
    ///        initialized on the next awake pass.
    ///
    /// @note  Best practice: call this from the constructor or on_awake(). For runtime spawning,
    ///        any time outside of the render pass is safe.
    ///
    /// @note  Ownership: store the returned @c std::weak_ptr as a member variable. Do not lock
    ///        and store as a @c shared_ptr - Entity owns the child and holds the @c shared_ptr
    ///        internally. Storing another @c shared_ptr will prevent correct destruction.
    ///
    /// @note  Tagging: pass @p tag to distinguish multiple children of the same type so they can
    ///        later be looked up with get_child_entity<T>(tag). Untagged children use "".
    ///
    /// @tparam TEntity The derived Entity type to construct. Must be derived from Entity.
    ///
    /// @param [in] config - Optional construction parameters forwarded to the TEntity constructor.
    /// @param [in] tag    - A string tag to distinguish multiple child Entities of the same type.
    ///                      Pass @c "" for untagged (default) children.
    ///
    /// @return A @c std::weak_ptr<TEntity> to the newly created child Entity.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename TEntity>
    requires(std::derived_from<TEntity, Entity>)
    std::weak_ptr<TEntity> add_child_entity(typename TEntity::Configuration config = {}, std::string_view tag = "")
    {
        on_component_added();
        auto child = std::make_shared<TEntity>(*this, config);
        m_pending_children.push_back(PendingChild{ child, std::string(tag) });
        return child;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Retrieves the child Entity of type TChildEntity with the given tag.
    ///
    /// @note  Only finds children that have completed their awake/start pass (i.e. have been
    ///        moved into the active child collection). A child added earlier in the same frame is
    ///        not queryable yet - prefer caching the @c weak_ptr returned by add_child_entity()
    ///        when you need to use it immediately.
    ///
    /// @note  Performance: O(1) hash-map lookup on fully-initialized children. Cache the result in
    ///        a member @c weak_ptr rather than calling this every frame.
    ///
    /// @tparam TChildEntity The Entity type to search for. Must be derived from Entity.
    ///
    /// @param [in] tag - Tag of the child Entity to retrieve. Defaults to @c "" (untagged).
    ///
    /// @return A @c std::weak_ptr<TChildEntity> to the matching child, or an empty
    ///         @c std::weak_ptr<TChildEntity> if no match is found.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename TChildEntity>
    requires(std::derived_from<TChildEntity, Entity>)
    std::weak_ptr<TChildEntity> get_child_entity(std::string_view tag = "") const
    {
        auto it = m_child_store.find(ComponentKey{ std::type_index(typeid(TChildEntity)), std::string(tag) });
        if (it != m_child_store.end())
        {
            return std::static_pointer_cast<TChildEntity>(it->second);
        }
        return std::weak_ptr<TChildEntity>();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Constructs and attaches a Component of type TComponent to this Entity. The
    ///        Component is owned by this Entity and initialized on the current or next awake pass.
    ///
    /// @note  Best practice: add Components in the constructor or on_awake(). Components added
    ///        during on_awake() are accessible to sibling Components before on_start() runs.
    ///
    /// @note  Ownership: store the returned @c std::weak_ptr as a member variable. Do not store
    ///        it as a @c shared_ptr - Entity owns the Component via a @c shared_ptr internally.
    ///        Storing an additional @c shared_ptr will prevent correct destruction.
    ///
    /// @note  Uniqueness: if @c TComponent::unique_per_entity is @c true, at most one Component
    ///        of that type can exist on this Entity regardless of tag. If @c false (the default),
    ///        multiple Components of the same type can coexist, distinguished by tag.
    ///
    /// @tparam TComponent The Component type to construct. Must be derived from Component.
    ///
    /// @param [in] tag    - A string tag to distinguish multiple Components of the same type on
    ///                      the same Entity. Pass @c "" for untagged (default) Components.
    /// @param [in] config - Optional construction parameters forwarded to the TComponent
    ///                      constructor.
    ///
    /// @return A @c std::weak_ptr<TComponent> to the newly created Component, or an empty
    ///         @c std::weak_ptr<TComponent> if a Component of that type (and tag) already exists.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename TComponent>
    requires(std::derived_from<TComponent, Component>)
    std::weak_ptr<TComponent> add_component(std::string_view tag, typename TComponent::Configuration config = {})
    {
        std::weak_ptr<TComponent> rtn{};
        bool const already_exists = [&]() -> bool
        {
            if constexpr (TComponent::unique_per_entity)
                return has_component_of_any_tag<TComponent>();
            else
                return has_component<TComponent>(tag);
        }();
        if (already_exists)
        {
            std::type_index const type{ typeid(TComponent) };
            std::println("[Entity::add_component] Component '{}' with tag '{}' already exists.", type.name(), tag);
        }
        else
        {
            on_component_added();
            auto component_ptr = std::make_shared<TComponent>(*this, config);
            m_pending_components.push_back(PendingComponent{ component_ptr, std::string(tag) });
            rtn = component_ptr;
        }
        return rtn;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Retrieves the Component of type TComponent with the given tag from this Entity.
    ///
    /// @note  Performance: this is an O(1) hash-map lookup on fully-initialized Components.
    ///        Avoid calling per-frame - cache the result in a member @c weak_ptr during on_awake()
    ///        or on_start() and reuse it.
    ///
    ///        Unity-pattern pitfall: calling get_component every frame is a common source of
    ///        performance regressions in engine code. Cache once, use many times.
    ///
    /// @note  Always check the result with @c .lock() before dereferencing:
    ///        @code
    ///        if (auto ptr = get_component<TransformComponent>().lock())
    ///        {
    ///            ptr->set_position(...);
    ///        }
    ///        @endcode
    ///
    /// @tparam TComponent The Component type to search for. Must be derived from Component.
    ///
    /// @param [in] tag - Tag of the Component to retrieve. Defaults to @c "" (untagged).
    ///
    /// @return A @c std::weak_ptr<TComponent> to the matching Component, or an empty
    ///         @c std::weak_ptr<TComponent> if no match is found.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename TComponent>
    requires(std::derived_from<TComponent, Component>)
    std::weak_ptr<TComponent> get_component(std::string_view tag = "") const
    {
        auto it = m_component_store.find(ComponentKey{ std::type_index(typeid(TComponent)), std::string(tag) });
        if (it != m_component_store.end())
        {
            return std::static_pointer_cast<TComponent>(it->second);
        }
        return std::weak_ptr<TComponent>();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Non-template overload of get_component<T>() used internally by Component to avoid
    ///        a circular header dependency. Prefer get_component<T>() at all other call sites.
    ///
    /// @param [in] type - The @c std::type_index of the desired Component type.
    ///
    /// @return A @c std::weak_ptr<Component> to the first Component of that type, or empty.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    std::weak_ptr<Component> get_component_by_type(std::type_index type) const;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Non-template overload of get_component<T>(tag) used internally by Component.
    ///        Prefer get_component<T>(tag) at all other call sites.
    ///
    /// @param [in] type - The @c std::type_index of the desired Component type.
    /// @param [in] tag  - Tag of the Component to retrieve.
    ///
    /// @return A @c std::weak_ptr<Component> to the matching Component, or empty if not found.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    std::weak_ptr<Component> get_component_by_type(std::type_index type, std::string_view tag) const;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Checks whether this Entity has a Component of type TComponent with the given tag.
    ///
    /// @note  Performance: active Component check is O(1). Pending Component check is O(n) over
    ///        the pending list, which is almost always tiny (< 10 entries in practice).
    ///
    /// @note  By default, also searches Components added but not yet awoken (pending). Set
    ///        @p search_pending to @c false to restrict to fully-active Components only.
    ///
    /// @tparam TComponent The Component type to search for. Must be derived from Component.
    ///
    /// @param [in] tag            - Tag of the Component to check for. Defaults to @c "".
    /// @param [in] search_pending - If @c true (default), also searches pending Components.
    ///
    /// @return @c true if a matching Component exists on this Entity.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename TComponent>
    requires(std::derived_from<TComponent, Component>)
    bool has_component(std::string_view tag = "", bool search_pending = true) const
    {
        std::type_index const type{ typeid(TComponent) };

        // Check pending components. O(n), but this list is typically tiny
        bool const is_pending = std::ranges::any_of(
            m_pending_components,
            [&type, &tag](PendingComponent const& pending)
            {
                return std::type_index{ typeid(*pending.component.get()) } == type
                    && pending.tag == tag;
            });

        // Check already-initialized components. O(1)
        bool const active = m_component_store.contains(ComponentKey{ type, std::string(tag) });

        // (is_pending or active) if search_pending, else (active)
        return search_pending ? (is_pending || active) : active;
    }

private:
    friend class Layer;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Engine Functions
    //
    // Called by the owning Layer to drive the Entity through its lifecycle. Access is restricted
    // to Layer via the friend declaration above - game-developer subclasses cannot call these.
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Phase 1 of Entity initialization. Flushes all pending Components into the active
    ///        store, calls on_awake(), and transitions state from Pending to Awoken.
    ///
    /// @note  Called by: Layer::initialize_entities() during the initialization pass at the start
    ///        of each frame.
    ///
    /// @note  Precondition: Entity must be in the Pending state.
    ///
    /// @note  Postcondition: Entity is in the Awoken state. All Components added during
    ///        construction are in the active store and have had on_awake() called. Any children
    ///        added during construction or on_awake() have also been awoken.
    ///
    /// @note  Not overridable. Override on_awake() to inject custom behavior.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void awake_entity();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Phase 2 of Entity initialization. Calls on_start() on all active Components and
    ///        on this Entity, then transitions state from Awoken to Active.
    ///
    /// @note  Called by: Layer::initialize_entities(), after all Entities in the same batch have
    ///        completed awake_entity().
    ///
    /// @note  Precondition: Entity must be in the Awoken state.
    ///
    /// @note  Postcondition: Entity is Active and will receive update_entity() every frame.
    ///
    /// @note  Not overridable. Override on_start() to inject custom behavior.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void start_entity();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Runs one frame of updates. Calls on_update() and on_late_update() on this Entity
    ///        and on all attached Components in update order.
    ///
    /// @note  Called by: Layer::update_entities() every frame.
    ///
    /// @note  Precondition: Entity must be Active; no-ops for all other states.
    ///
    /// @note  Call order within one frame: awake_components() -> Component::on_update() (sorted)
    ///        -> on_update() -> Component::on_late_update() (sorted) -> on_late_update() ->
    ///        flush_pending_children() -> update_children().
    ///
    /// @note  Not overridable. Override on_update() and/or on_late_update() for custom behavior.
    ///
    /// @param [in] delta_time - Seconds elapsed since the last frame.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void update_entity(float delta_time);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Runs one fixed-rate update. Calls on_fixed_update() on this Entity and on all
    ///        attached Components.
    ///
    /// @note  Called by: Layer::fixed_update_entities() at a fixed timestep.
    ///
    /// @note  Precondition: Entity must be Active; no-ops for all other states.
    ///
    /// @note  Not overridable. Override on_fixed_update() for custom behavior.
    ///
    /// @param [in] fixed_dt - The fixed timestep in seconds.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void fixed_update_entity(float fixed_dt);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Renders this Entity. Calls on_render() on all attached Components and on this
    ///        Entity, then recurses into all child Entities. Runs for Active and Inactive Entities.
    ///
    /// @note  Called by: Layer::render_entities() every frame.
    ///
    /// @note  Precondition: Entity must be Active or Inactive; no-ops for Pending, Awoken, and
    ///        Destroyed states.
    ///
    /// @note  Not overridable. Override on_render() for custom render behavior.
    ///
    /// @param [in] renderer - The SDL renderer for the current frame.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void render_entity(SDL_Renderer* renderer);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Cleans up this Entity before destruction. Calls on_cleanup() on all children,
    ///        on this Entity, then on all attached Components.
    ///
    /// @note  Called by: Layer::update_entities() when the Entity is Destroyed, or
    ///        Layer::cleanup_entities() during Layer shutdown.
    ///
    /// @note  Not overridable. Override on_cleanup() for custom teardown logic.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void cleanup_entity();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Propagates an event down to all Components on this Entity, then to this Entity's
    ///        own on_event(), then to all child Entities. Stops early if the event is handled.
    ///
    /// @note  Called by: Layer::propagate_event_down() during event dispatch.
    ///
    /// @param [in] event - The event being propagated.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void propagate_event_down(Event& event);


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Private Internal Types
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Selects whether to sort by update_order or render_order during Component insertion.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    enum class EComponentInsertType
    {
        Update,
        Render,
    };

    /// @brief Key type for the component store map: (type_index, tag) pair.
    using ComponentKey = std::pair<std::type_index, std::string>;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Hash functor for ComponentKey.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    struct ComponentKeyHash
    {
        std::size_t operator()(ComponentKey const& k) const noexcept
        {
            std::size_t h1 = std::hash<std::type_index>{}(k.first);
            std::size_t h2 = std::hash<std::string>{}(k.second);
            return h1 ^ (h2 << 1);
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Holds a not-yet-awoken Component and its associated tag.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    struct PendingComponent
    {
        std::shared_ptr<Component> component;
        std::string tag;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Holds a not-yet-awoken child Entity and its associated tag.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    struct PendingChild
    {
        std::shared_ptr<Entity> entity;
        std::string tag;
    };


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Private Component Lifecycle Helpers
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Flushes all pending Components into the active store and calls on_awake() on each.
    ///        Two-pass: all Components enter the store before any virtual code runs, so siblings
    ///        are always reachable via get_component<T>() during on_awake(). When the Entity is
    ///        already Active, newly-added Components are also immediately started.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void awake_components();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Calls on_start() on every active Component. Invoked from start_entity() and,
    ///        for runtime-added Components, immediately after awake_components().
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void start_components();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Calls on_update() on all active Components in update_order sequence.
    ///
    /// @param [in] delta_time - Seconds elapsed since the last frame.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void update_components(float delta_time);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Calls on_late_update() on all active Components in update_order sequence.
    ///
    /// @param [in] delta_time - Seconds elapsed since the last frame.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void late_update_components(float delta_time);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Calls on_fixed_update() on all active Components.
    ///
    /// @param [in] fixed_dt - The fixed timestep in seconds.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void fixed_update_components(float fixed_dt);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Calls on_render() on all active Components in render_order sequence.
    ///
    /// @param [in] renderer - The SDL renderer for the current frame.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void render_components(SDL_Renderer* renderer);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Calls on_cleanup() on all active Components.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void cleanup_components();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Asserts that neither the render pass nor the cleanup pass is in progress. Called
    ///        before adding any Component or child Entity to enforce safe mutation timing.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void on_component_added() const;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Checks whether any Component of type TComponent exists on this Entity, regardless
    ///        of tag. Used to enforce the unique_per_entity constraint in add_component<T>().
    ///
    /// @tparam TComponent The Component type to check. Must be derived from Component.
    ///
    /// @param [in] search_pending - If @c true (default), also searches pending Components.
    ///
    /// @return @c true if any Component of type TComponent is found.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename TComponent>
    requires(std::derived_from<TComponent, Component>)
    bool has_component_of_any_tag(bool search_pending = true) const
    {
        std::type_index const type{ typeid(TComponent) };

        if (search_pending)
        {
            bool const is_pending = std::ranges::any_of(
                m_pending_components,
                [&type](PendingComponent const& pending)
                {
                    return std::type_index{ typeid(*pending.component.get()) } == type;
                });
            if (is_pending) return true;
        }

        return std::ranges::any_of(
            m_component_store,
            [&type](auto const& kv) { return kv.first.first == type; });
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Inserts a Component into a sorted Component collection. Components are maintained
    ///        in ascending order of their update_order or render_order value (determined by
    ///        @p insert_sorter). Called for both the update and render collections.
    ///
    /// @param [in] component_collection - The collection to insert into.
    /// @param [in] component            - The Component to insert.
    /// @param [in] insert_sorter        - Selects the sort key (Update or Render order).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void insert_component_sorted(std::vector<std::weak_ptr<Component>>& component_collection,
                                 std::weak_ptr<Component> component,
                                 EComponentInsertType insert_sorter);


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Private Child Entity Helpers
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Two-pass initialization (awake then start) over all pending children. Called from
    ///        update_entity() so that children added at runtime are initialized before their
    ///        first update.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void flush_pending_children();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Calls update_entity() on all active child Entities. Removes children that are
    ///        in the Destroyed state after their update.
    ///
    /// @param [in] delta_time - Seconds elapsed since the last frame.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void update_children(float delta_time);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Calls render_entity() on all active child Entities.
    ///
    /// @param [in] renderer - The SDL renderer for the current frame.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void render_children(SDL_Renderer* renderer);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Calls fixed_update_entity() on all active child Entities.
    ///
    /// @param [in] fixed_dt - The fixed timestep in seconds.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void fixed_update_children(float fixed_dt);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Calls cleanup_entity() on all child Entities and clears both child collections.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void cleanup_children();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Propagates an event to all child Entities via their propagate_event_down(). Stops
    ///        early if the event is marked as handled.
    ///
    /// @param [in] event - The event being propagated.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void propagate_event_to_children(Event& event);


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Private Member Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    /// @brief The Layer that owns this Entity.
    Layer& m_owning_layer;
    
    /// @brief Parent Entity, or nullptr for root Entities.
    Entity* m_parent{ nullptr };

    /// @brief Update order relative to sibling Entities on the same Layer.
    std::uint8_t m_update_order;

    /// @brief Current lifecycle state.
    EState m_state{ EState::Pending };

    /// @brief Children awaiting initialization.
    std::vector<PendingChild> m_pending_children{};
    
    /// @brief Active child Entities, sorted by update_order.
    std::vector<std::shared_ptr<Entity>> m_children{};

    /// @brief Active child Entity store, keyed by (type, tag). Populated alongside m_children, so
    ///        it only ever contains fully awoken-and-started children. Reuses ComponentKey /
    ///        ComponentKeyHash since both are just generic (type_index, tag) lookups.
    std::unordered_map<ComponentKey, std::shared_ptr<Entity>, ComponentKeyHash> m_child_store{};

    /// @brief Components awaiting awake_components().
    std::vector<PendingComponent> m_pending_components{};
    
    /// @brief Active Component store, keyed by (type, tag).
    std::unordered_map<ComponentKey, std::shared_ptr<Component>, ComponentKeyHash> m_component_store{};

    /// @brief Components sorted by update_order.
    std::vector<std::weak_ptr<Component>> m_update_ordered_components{};

    /// @brief Components sorted by render_order.
    std::vector<std::weak_ptr<Component>> m_render_ordered_components{};
};

} // namespace Core

#endif // CORE_ENTITY_H
