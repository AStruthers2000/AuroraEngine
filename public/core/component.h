////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Base Component class. Components are owned by Entities and provide specializations of
///        functionality.
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
/// @brief Base Component class. Components are owned by Entities and provide specializations of
///        functionality. Override awake(), start(), update(), and/or render() for custom behavior.
////////////////////////////////////////////////////////////////////////////////////////////////////
class Component
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Mapping of different orderings. Lower values update first.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    struct Order
    {
        /// @brief Sorting order for this component's update() call relative to other
        ///        Components owned by the parent Entity.
        std::uint8_t update_order = DEFAULT_SORTING_ORDER;

        /// @brief Sorting order for this component's render() call relative to other
        ///        Components owned by the parent Entity./
        std::uint8_t render_order = DEFAULT_SORTING_ORDER;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief When true, the owning Entity enforces at most one Component of this derived type,
    ///        regardless of tag. Set to true in a derived Component class to prevent duplicates.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static constexpr bool unique_per_entity = false;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Component constructor. Default sorting order.
    ///
    /// @param [in] owning_entity - The Entity that owns this Component instance.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    explicit Component(Entity& owning_entity);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Component constructor. Custom sorting order.
    ///
    /// @param [in] owning_entity   - The Entity that owns this Component instance. 
    /// @param [in] component_order - The sorting order mapping this Component will follow.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    Component(Entity& owning_entity, Order const& component_order);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Component destructor.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~Component();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Broadcasts an event to the Event system.
    ///
    /// @param [in] event - Event to be broadcasted.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void broadcast_event(Event& event);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Broadcasts an event within the owning Entity only, without propagating to the Engine
    ///        root. Equivalent to calling broadcast_event_within_entity() on the owning Entity.
    ///        Useful for notifying sibling Components without a global round-trip.
    ///
    /// @param [in] event - Event to be broadcast within the owning Entity.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void broadcast_event_within_entity(Event& event);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Runs any Event-handling-specific code. Allows Components to dispatch Events. Called
    ///        from Entity::propagate_event_down(). Overridable.
    ///
    /// @param [in] event - Event that was broadcast to the Event system.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void on_event(Event& event);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Phase 1 of Component initialization. Called from owning Entity after all pending
    ///        Components have been flushed to the active store. Calls awake(). Not overridable.
    ///
    /// @note It is safe to call get_sibling_component<T>() here. All Components added during Entity
    ///       construction are in the active store by the time this runs.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void awake_component();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Runs any Component-specific Phase 1 initialization code. Called from
    ///        Component::awake_component(). Overridable.
    ///
    /// @note Contract: all Components added during Entity construction are accessible via
    ///       get_sibling_component<T>(). Do not access Components on other Entities here; their
    ///       owning Entity's awake_entity() is not guaranteed to have run yet.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void awake();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Phase 2 of Component initialization. Called from owning Entity during
    ///        Entity::start_entity(), after all sibling Entities in the same initialization pass
    ///        have awoken. Calls start(). Not overridable.
    ///
    /// @note It is safe to call get_sibling_component<T>() on other Entities here.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void start_component();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Runs any Component-specific Phase 2 initialization code. Called from
    ///        Component::start_component(). Overridable.
    ///
    /// @note Contract: all Entities that were pending in the same initialization pass have
    ///       completed their awake phase, so their Components are fully initialized and accessible
    ///       via get_sibling_component<T>() on the owning Entity.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void start();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Updates this Component. Called from owning Entity. Not overridable.
    ///
    /// @param [in] delta_time - Time since last update. 
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void update_component(float delta_time);
    
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Runs any Component-specific update code. Called from
    ///        Entity::update_components(). Overridable.
    ///
    /// @param [in] delta_time - Time since last update. 
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void update(float delta_time);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Renders this Component. Called from owning Entity. Not overridable.
    ///
    /// @param [in] renderer - Renderer provided by the owning Entity. 
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void render_component(SDL_Renderer* renderer);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Runs any Component-specific render code. Called from
    ///        Entity::render_components(). Overridable.
    /// 
    /// @param [in] renderer - Renderer provided by the owning Entity. 
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void render(SDL_Renderer* renderer);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Late-updates this Component. Called from the owning Entity after Entity::update()
    ///        and Entity::late_update() have run. Not overridable.
    ///
    /// @param [in] delta_time - Time since last update.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void late_update_component(float delta_time);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Runs any Component-specific late-update code. Called from
    ///        Component::late_update_component(). Overridable.
    ///
    /// @note Executes after Entity::update() and Entity::late_update() have run. Useful for
    ///       components that must read fully-resolved Entity state for this frame (e.g. a camera
    ///       component reading a finalized position).
    ///
    /// @param [in] delta_time - Time since last update.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void late_update(float delta_time);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Fixed-rate updates this Component. Called from the owning Entity at a fixed
    ///        timestep independent of frame rate. Not overridable.
    ///
    /// @param [in] fixed_dt - The fixed timestep interval.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void fixed_update_component(float fixed_dt);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Runs any Component-specific fixed-rate update code. Called from
    ///        Component::fixed_update_component(). Overridable.
    ///
    /// @param [in] fixed_dt - The fixed timestep interval.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void fixed_update(float fixed_dt);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Cleans up this Component. Called from the owning Entity before the Entity is
    ///        destroyed. Not overridable.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void cleanup_component();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Runs any Component-specific cleanup code. Called from
    ///        Component::cleanup_component(). Overridable.
    ///
    /// @note Entity::cleanup() has already run by the time this is called. Do not call
    ///       add_component() or similar engine-mutating methods here.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void cleanup();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return Gets the desired update() sorting order.
    //////////////////////////////////////////////////////////////////////////////////////////////// 
    std::uint8_t get_update_order() const { return m_order.update_order; }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return Gets the desired render() sorting order.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    std::uint8_t get_render_order() const { return m_order.render_order; }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return Gets the Entity that owns this Component.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    Entity& get_owner() const { return m_owner; }

protected:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Tries to get the Component of type TComponent owned by the same Entity as this
    ///        Component. Delegates to the owning Entity's component store.
    ///
    /// @tparam TComponent Component type to search for. Must be derived from Component.
    ///
    /// @return A std::weak_ptr<TComponent> to the sibling Component, or an empty
    ///         std::weak_ptr<TComponent> if no such Component exists on the owning Entity.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename TComponent>
    requires(std::derived_from<TComponent, Component>)
    std::weak_ptr<TComponent> get_sibling_component(std::string_view tag = "") const
    {
        return std::static_pointer_cast<TComponent>(
            get_sibling_component_impl(std::type_index(typeid(TComponent)), tag).lock());
    }

private:
    std::weak_ptr<Component> get_sibling_component_impl(std::type_index type, std::string_view tag) const;

    Entity& m_owner;
    Order m_order;
};

} // namespace Core

#endif // CORE_COMPONENT_H
