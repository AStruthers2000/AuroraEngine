////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Base Component class. Components are owned by Entities and encapsulate discrete pieces
///        of functionality.
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CORE_COMPONENT_H
#define CORE_COMPONENT_H

#include "core/layer.h"

#include <memory>
#include <string_view>
#include <typeindex>

namespace Core
{

class Entity;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Base Component class. Components are owned by Entities and encapsulate discrete pieces
///        of functionality.
///
/// A Component is a self-contained unit of behavior or data attached to an Entity. Rather than
/// building monolithic Entity subclasses, decompose behavior into Components: a TransformComponent
/// holds position and scale, a RectRenderComponent draws a rectangle, and so on.
///
/// To create a Component, derive from Component and override the virtual lifecycle hooks
/// (on_awake, on_start, on_update, on_render, etc.). Access sibling Components on the same
/// Entity via get_sibling_component<T>() - cache the returned std::weak_ptr during on_awake()
/// rather than calling it every frame.
////////////////////////////////////////////////////////////////////////////////////////////////////
class Component
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Types
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Sorting orders for on_update() and on_render() relative to other Components on the
    ///        same Entity. Lower values run first.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    struct Order
    {
        /// @brief Determines when this Component's on_update() runs relative to other Components
        ///        on the parent Entity. Lower values run first. Defaults to DEFAULT_SORTING_ORDER.
        std::uint8_t update_order = DEFAULT_SORTING_ORDER;

        /// @brief Determines when this Component's on_render() runs relative to other Components
        ///        on the parent Entity. Lower values render first. Defaults to DEFAULT_SORTING_ORDER.
        std::uint8_t render_order = DEFAULT_SORTING_ORDER;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Construction parameters for Component and all derived classes.
    ///
    /// Derived classes should inherit from Component::Configuration and add their own fields:
    /// @code
    /// struct Configuration : Component::Configuration
    /// {
    ///     SDL_Color color{ 255, 255, 255, 255 };
    /// };
    /// @endcode
    ////////////////////////////////////////////////////////////////////////////////////////////////
    struct Configuration
    {
        Order order{};
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief When @c true, the owning Entity enforces at most one Component of this derived type,
    ///        regardless of tag. Override this in a derived Component class to prevent duplicates:
    ///
    ///        @code
    ///        class TransformComponent : public Component
    ///        {
    ///        public:
    ///            static constexpr bool unique_per_entity = true;
    ///        };
    ///        @endcode
    ///
    ///        When @c false (the default), multiple Components of the same derived type can
    ///        coexist on one Entity, distinguished by tag.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static constexpr bool unique_per_entity = false;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Construction & Destruction
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Component constructor.
    ///
    /// @param [in] owner  - The Entity that owns this Component.
    /// @param [in] config - Optional construction parameters including update/render order.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    explicit Component(Entity& owner, Configuration const& config = {});

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Component destructor. Virtual to support polymorphic deletion.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~Component();


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Virtual Lifecycle Hooks
    //
    // Override these to implement custom Component behavior. The engine calls them through the
    // non-virtual orchestrators (awake_component, update_component, etc.) - override these hooks,
    // not those orchestrators.
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Phase 1 initialization hook. Called once after all Components added during Entity
    ///        construction have been flushed to the active store.
    ///
    /// @note  Contract: all sibling Components added during Entity construction are accessible via
    ///        get_sibling_component<T>(). Cache weak_ptr references to siblings here.
    ///
    /// @note  Must not: access Components on other Entities - their owning Entity's on_awake() is
    ///        not guaranteed to have run yet. Use on_start() for cross-Entity setup.
    ///
    /// @note  Use this override to:
    ///         - Cache weak_ptr references to sibling Components for per-frame use.
    ///         - Initialize state that depends on sibling Component presence.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void on_awake() {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Phase 2 initialization hook. Called once after all Entities in the same
    ///        initialization pass have completed on_awake().
    ///
    /// @note  Contract: all Entities pending in the same batch have completed on_awake(). It is
    ///        safe to access other Entities and their Components.
    ///
    /// @note  Use this override to:
    ///         - Set up state that depends on other Entities having been initialized.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void on_start() {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Per-frame update hook. Called every frame while the owning Entity is Active.
    ///
    /// @note  Runs in update_order sequence relative to sibling Components on the same Entity.
    ///
    /// @param [in] delta_time - Seconds elapsed since the last frame.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void on_update(float delta_time) {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Per-frame late-update hook. Called after the owning Entity's on_update() and after
    ///        all Component on_update() calls have completed.
    ///
    /// @note  Use this to read fully-resolved per-frame Entity state (e.g. a camera component
    ///        that must see a finalized position before computing its view matrix).
    ///
    /// @param [in] delta_time - Seconds elapsed since the last frame.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void on_late_update(float delta_time) {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Fixed-rate update hook. Called at a fixed timestep, independent of frame rate.
    ///
    /// @note  Use this for physics or simulation logic that requires a stable timestep.
    ///
    /// @param [in] fixed_dt - The fixed timestep in seconds.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void on_fixed_update(float fixed_dt) {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Per-frame render hook. Called every frame for Active and Inactive Entities.
    ///
    /// @note  Must not: mutate engine state. Render passes are read-only.
    ///
    /// @param [in] renderer - The SDL renderer for the current frame.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void on_render(SDL_Renderer* renderer) {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Cleanup hook. Called once before the owning Entity is destroyed.
    ///
    /// @note  The owning Entity's on_cleanup() has already run by the time this is called.
    ///
    /// @note  Must not: call add_component<T>() or similar engine-mutating methods.
    ///
    /// @note  Use this to release resources that require explicit teardown (e.g. GPU textures).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void on_cleanup() {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Event hook. Called when an event is propagated down to the owning Entity.
    ///        Components receive the event before the Entity's own on_event().
    ///
    /// @note  Use an EventDispatcher inside this override to route events to typed handler
    ///        lambdas. Return @c true from a dispatch handler to mark the event handled and stop
    ///        propagation.
    ///
    /// @param [in] event - The event being propagated.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void on_event(Event& event) {}


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Events
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Broadcasts an event engine-wide. Equivalent to calling broadcast_event() on the
    ///        owning Entity.
    ///
    /// @param [in] event - The event to broadcast.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void broadcast_event(Event& event);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Broadcasts an event within the owning Entity only. Useful for notifying sibling
    ///        Components without a full engine-wide round-trip. Equivalent to calling
    ///        broadcast_event_within_entity() on the owning Entity.
    ///
    /// @param [in] event - The event to broadcast within the owning Entity.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void broadcast_event_within_entity(Event& event);


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Queries
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return A reference to the Entity that owns this Component.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    Entity& get_owner() const { return m_owner; }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return The update order of this Component relative to siblings on the same Entity.
    ///         Lower values update first.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    std::uint8_t get_update_order() const { return m_order.update_order; }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return The render order of this Component relative to siblings on the same Entity.
    ///         Lower values render first.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    std::uint8_t get_render_order() const { return m_order.render_order; }

protected:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Retrieves a sibling Component of type TComponent (another Component on the same
    ///        Entity) with the given tag.
    ///
    /// @note  Performance: delegates to the owning Entity's get_component<T>(), an O(1) lookup.
    ///        Avoid calling per-frame - cache the result in a member weak_ptr during on_awake().
    ///
    /// @note  Only accessible from derived Component classes (protected). From external code,
    ///        call get_component<T>() on the owning Entity directly.
    ///
    /// @note  Usage:
    ///        @code
    ///        // In on_awake():
    ///        m_transform = get_sibling_component<TransformComponent>();
    ///
    ///        // In on_update():
    ///        if (auto ptr = m_transform.lock()) { ... }
    ///        @endcode
    ///
    /// @tparam TComponent The Component type to search for. Must be derived from Component.
    ///
    /// @param [in] tag - Tag of the Component to retrieve. Defaults to @c "".
    ///
    /// @return A @c std::weak_ptr<TComponent> to the sibling, or empty if not found.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename TComponent>
    requires(std::derived_from<TComponent, Component>)
    std::weak_ptr<TComponent> get_sibling_component(std::string_view tag = "") const
    {
        return std::static_pointer_cast<TComponent>(
            get_sibling_component_impl(std::type_index(typeid(TComponent)), tag).lock());
    }

private:
    friend class Entity;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Engine Functions
    //
    // Called by the owning Entity to drive the Component through its lifecycle. Access is
    // restricted to Entity via the friend declaration above.
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Calls on_awake() on this Component. Called from Entity::awake_components() after
    ///        all pending siblings have been flushed to the active store.
    ///
    /// @note  Called by: Entity::awake_components().
    /// @note  Not overridable. Override on_awake() to inject custom behavior.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void awake_component();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Calls on_start() on this Component. Called from Entity::start_components() after
    ///        all sibling Entities in the same initialization pass have awoken.
    ///
    /// @note  Called by: Entity::start_components().
    /// @note  Not overridable. Override on_start() to inject custom behavior.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void start_component();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Calls on_update() on this Component.
    ///
    /// @note  Called by: Entity::update_components() every frame.
    /// @note  Not overridable. Override on_update() to inject custom behavior.
    ///
    /// @param [in] delta_time - Seconds elapsed since the last frame.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void update_component(float delta_time);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Calls on_late_update() on this Component. Called after all Component on_update()
    ///        calls and after Entity::on_update() and Entity::on_late_update() have run.
    ///
    /// @note  Called by: Entity::late_update_components() every frame.
    /// @note  Not overridable. Override on_late_update() for custom behavior.
    ///
    /// @param [in] delta_time - Seconds elapsed since the last frame.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void late_update_component(float delta_time);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Calls on_fixed_update() on this Component.
    ///
    /// @note  Called by: Entity::fixed_update_components() at a fixed timestep.
    /// @note  Not overridable. Override on_fixed_update() for custom behavior.
    ///
    /// @param [in] fixed_dt - The fixed timestep in seconds.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void fixed_update_component(float fixed_dt);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Calls on_render() on this Component.
    ///
    /// @note  Called by: Entity::render_components() every frame.
    /// @note  Not overridable. Override on_render() for custom render behavior.
    ///
    /// @param [in] renderer - The SDL renderer for the current frame.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void render_component(SDL_Renderer* renderer);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Calls on_cleanup() on this Component. Called from Entity::cleanup_components()
    ///        as part of Entity teardown.
    ///
    /// @note  Called by: Entity::cleanup_components().
    /// @note  Not overridable. Override on_cleanup() for custom teardown logic.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void cleanup_component();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Non-virtual implementation of get_sibling_component<T>(). Delegates to the owning
    ///        Entity's get_component_by_type() to avoid a template instantiation in this header.
    ///
    /// @param [in] type - The std::type_index of the desired sibling Component type.
    /// @param [in] tag  - Tag of the Component to retrieve.
    ///
    /// @return A std::weak_ptr<Component> to the matching sibling, or empty if not found.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    std::weak_ptr<Component> get_sibling_component_impl(std::type_index type, std::string_view tag) const;

    /// @brief The Entity that owns this Component.
    Entity& m_owner;

    /// @brief Update and render sort orders for this Component.
    Order   m_order;
};

} // namespace Core

#endif // CORE_COMPONENT_H
