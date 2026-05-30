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
#include <typeindex>

namespace Core
{

class Entity;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Base Component class. Components are owned by Entities and provide specializations of
///        functionality. Override awake_component(), start_component(), update_component(), and/or
///        render_component() for custom behavior.
////////////////////////////////////////////////////////////////////////////////////////////////////
class Component
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Mapping of different orderings. Lower values update first.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    struct Order
    {
        /// @brief Sorting order for this component's update_component() call relative to other
        ///        Components owned by the parent Entity.
        std::uint8_t update_order = DEFAULT_SORTING_ORDER;

        /// @brief Sorting order for this component's render_component() call relative to other
        ///        Components owned by the parent Entity./
        std::uint8_t render_order = DEFAULT_SORTING_ORDER;
    };

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
    /// @brief Phase 1 of Component initialization. Called from owning Entity after all pending
    ///        Components have been flushed to the active store. Calls awake_component(). Not
    ///        overridable.
    ///
    /// @note It is safe to call get_sibling_component<T>() here. All Components added during Entity
    ///       construction are in the active store by the time this runs.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void awake();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Runs any Component-specific Phase 1 initialization code. Called from
    ///        Component::awake(). Overridable.
    ///
    /// @note Contract: all Components added during Entity construction are accessible via
    ///       get_sibling_component<T>(). Do not access Components on other Entities here; their
    ///       owning Entity's awake() is not guaranteed to have run yet.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void awake_component();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Phase 2 of Component initialization. Called from owning Entity during
    ///        Entity::start(), after all sibling Entities in the same initialization pass have
    ///        awoken. Calls start_component(). Not overridable.
    ///
    /// @note It is safe to call get_sibling_component<T>() on other Entities here.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void start();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Runs any Component-specific Phase 2 initialization code. Called from
    ///        Component::start(). Overridable.
    ///
    /// @note Contract: all Entities that were pending in the same initialization pass have
    ///       completed awake(), so their Components are fully initialized and accessible via
    ///       get_sibling_component<T>() on the owning Entity.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void start_component();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Updates this Component. Called from owning Entity. Not overridable.
    ///
    /// @param [in] delta_time - Time since last update. 
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void update(float delta_time);
    
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Runs any Component-specific update code. Called from Entity::update_components().
    ///        Overridable.
    ///
    /// @param [in] delta_time - Time since last update. 
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void update_component(float delta_time);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Renders this Component. Called from owning Entity. Not overridable.
    ///
    /// @param [in] renderer - Renderer provided by the owning Entity. 
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void render(SDL_Renderer* renderer);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Runs any Component-specific render code. Called from Entity::render_components().
    ///        Overridable.
    /// 
    /// @param [in] renderer - Renderer provided by the owning Entity. 
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void render_component(SDL_Renderer* renderer);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return Gets the desired update_component() sorting order.
    //////////////////////////////////////////////////////////////////////////////////////////////// 
    std::uint8_t get_update_order() const { return m_order.update_order; }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return Gets the desired render_component() sorting order.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    std::uint8_t get_render_order() const { return m_order.render_order; }

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
    std::weak_ptr<TComponent> get_sibling_component() const
    {
        return std::static_pointer_cast<TComponent>(
            get_sibling_component_impl(std::type_index(typeid(TComponent))).lock());
    }

private:
    std::weak_ptr<Component> get_sibling_component_impl(std::type_index type) const;

    Entity& m_owner;
    Order m_order;
};

} // namespace Core

#endif // CORE_COMPONENT_H
