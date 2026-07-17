////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Base Entity class. Root of all objects in a Layer.
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CORE_ENTITY_H
#define CORE_ENTITY_H

#include "core/component.h"
#include "core/layer.h"
#include "core/components/transform_component.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cstdint>
#include <print>
#include <ranges>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Core
{

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Base Entity class. Root of all objects in a Layer. Override awake(), start(),
///        update(), and/or render() for custom behavior.
////////////////////////////////////////////////////////////////////////////////////////////////////
class Entity
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief State of the Entity. Controls how the Entity is updated.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    enum class EState
    {
        Active,     ///< Entity is in the "normal" state; Entity::update_entity() is called every tick.
        Inactive,   ///< Entity is paused. Entity::update_entity() will not be called, but the Entity
                    ///< can return to the Active state with no overhead.
        Pending,    ///< Entity has been constructed but not yet awoken. On the next initialization
                    ///< pass, Entity::awake_entity() will be called, transitioning to Awoken.
        Awoken,     ///< Entity has completed its awake phase but has not yet started. All Components
                    ///< are initialized and accessible. Entity::start_entity() will be called once
                    ///< all sibling Entities in the owning Layer have also awoken.
        Destroyed,  ///< Entity has been marked for destruction. Entity::~Entity() will be called on
                    ///< the next update cycle
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Entity constructor.
    ///
    /// @param [in] owning_layer - Entity must know what Layer owns it. 
    ////////////////////////////////////////////////////////////////////////////////////////////////
    Entity(Layer& owning_layer, std::uint8_t update_order = DEFAULT_SORTING_ORDER);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Entity destructor.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~Entity() = default;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Phase 1 of Entity initialization. Flushes all pending Components into the Component
    ///        store and calls awake(). Transitions state from Pending to Awoken. Not overridable.
    ///
    /// @note Called by the owning Layer before start_entity(). Guaranteed to be called before any
    ///       Entity in the Layer calls start_entity(), which means awake() overrides must not assume
    ///       other Entities have awoken yet.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void awake_entity();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Runs any Entity-specific Phase 1 initialization code. Called from
    ///        Entity::awake_entity(). Overridable.
    ///
    /// @note Contract: all Components added during construction are accessible via
    ///       get_component<T>(). Do not access Components or state on other Entities here; their
    ///       awake_entity() is not guaranteed to have run yet.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void awake();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Phase 2 of Entity initialization. Calls start() and transitions state from Awoken
    ///        to Active. Not overridable.
    ///
    /// @note Called by the owning Layer after every pending Entity in the same initialization pass
    ///       has completed awake_entity(). It is safe to access other Entities and their Components
    ///       here.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void start_entity();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Runs any Entity-specific Phase 2 initialization code. Called from
    ///        Entity::start_entity(). Overridable.
    ///
    /// @note Contract: all Entities that were pending in the same initialization pass have
    ///       completed their awake phase. It is safe to access other Entities and their Components.
    ///       Use this override, rather than awake(), for any setup that requires references to
    ///       other Entities or their Components.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void start();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Updates this Entity. Called from owning Layer. Not overridable.
    ///
    /// @note Also runs late_update() on the Entity and late_update_component() on all attached
    ///       Components after the main update pass completes.
    ///
    /// @param [in] delta_time - Time since last update. 
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void update_entity(float delta_time);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Runs any Entity-specific update code. Called from Entity::update_entity().
    ///        Overridable.
    ///
    /// @param [in] delta_time - Time since last update. 
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void update(float delta_time);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Renders this Entity. Called from owning Layer. Not overridable.
    ///
    /// @note Active and Inactive entities both render. Only Pending, Awoken, and Destroyed
    ///       entities are skipped.
    ///
    /// @param [in] renderer - Renderer provided by the owning Layer. 
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void render_entity(SDL_Renderer* renderer);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Runs any Entity-specific render code. Called from Entity::render_entity().
    ///        Overridable.
    ///
    /// @note Must not call destroy_entity(), add_component(), or any other state-mutating
    ///       engine method. Rendering must be a read-only pass over Entity state.
    ///
    /// @param [in] renderer - Renderer provided by the owning Layer. 
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void render(SDL_Renderer* renderer);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Sets this Entity to the Active state, allowing update_entity() to run. Has no
    ///        effect if the Entity is in the Pending or Destroyed state.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void set_active();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Sets this Entity to the Inactive state. While inactive, update_entity() is skipped
    ///        but render_entity() continues to run. Has no effect if the Entity is in the Pending
    ///        or Destroyed state.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void set_inactive();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Runs any Entity-specific late-update code. Called from Entity::update_entity(),
    ///        after update(). Overridable.
    ///
    /// @note Executes before component late_update(). Use this to finalize per-Entity state
    ///       before attached Components read it in their late_update() (e.g. a camera component
    ///       that must see a fully-resolved position).
    ///
    /// @param [in] delta_time - Time since last update.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void late_update(float delta_time);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Fixed-rate updates this Entity. Called from owning Layer at a fixed timestep
    ///        independent of frame rate. Not overridable.
    ///
    /// @param [in] fixed_dt - The fixed timestep interval.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void fixed_update_entity(float fixed_dt);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Runs any Entity-specific fixed-rate update code. Called from
    ///        Entity::fixed_update_entity(). Overridable.
    ///
    /// @param [in] fixed_dt - The fixed timestep interval.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void fixed_update(float fixed_dt);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Cleans up this Entity. Called by the owning Layer before the Entity is destroyed.
    ///        Not overridable.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void cleanup_entity();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Runs any Entity-specific cleanup code. Called from Entity::cleanup_entity().
    ///        Overridable.
    ///
    /// @note All Components are still accessible during this call. Component cleanup runs
    ///       after this returns. Do not call add_component() or destroy_entity() here.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void cleanup();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Marks this Entity for destruction. Destruction occurs at the end of this frame.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void destroy_entity();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Adds a Component to this Entity. Constructs a Component of type TComponent,
    ///        forwarding the arguments passed in args. Entity assumes ownership of the Component.
    ///        If there is already a component of type TComponent as part of this Entity (either
    ///        pending or active), this function will do nothing; dynamic memory allocation (i.e.
    ///        `std::make_shared`) only occurs after the Entity is checked for another active or
    ///        pending Component of the same type.
    ///
    /// @tparam TComponent Templated Component type to construct and attach. Requires that this type
    ///                    is derived from Component.
    /// @tparam ...Args    Types of arguments to forward to the TComponent constructor;
    ///                    automatically deduced.
    ///
    /// @param [in] args - Arguments forwarded to the TComponent constructor.
    ///
    /// @return Returns a std::weak_ptr<TComponent> to the newly created Component (safe for
    ///         long-term storage), or an empty std:::weak_ptr<TComponent> if a Component of that
    ///         type already exists (pending or active) on this Entity.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename TComponent, typename... Args>
    requires(std::derived_from<TComponent, Component>)
    std::weak_ptr<TComponent> add_component(Args&&... args)
    {
        std::weak_ptr<TComponent> rtn{};
        if (has_component<TComponent>())
        {
            std::type_index const type{ typeid(TComponent) };
            std::println("[Entity::add_component] Component '{}' already exists.", type.name());
        }
        else
        {
            on_component_added();
            auto component_ptr = std::make_shared<TComponent>(*this, std::forward<Args>(args)...);
            m_pending_components.push_back(component_ptr);
            rtn = component_ptr;
        }
        return rtn;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief 
    /// @param component 
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // void remove_component(std::unique_ptr<Component> component);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Tries to get the component of type TComponent owned by this Entity. Uses RTTI to
    ///        perform a O(1) lookup.
    ///
    /// @tparam TComponent Templated Component type to search for. Requires that this type is
    ///                    derived from Component.
    ///
    /// @return Returns a raw pointer to the Component of type TComponent, or an empty
    ///         std:::weak_ptr<TComponent> if no Component of that type was found.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename TComponent>
    requires(std::derived_from<TComponent, Component>)
    std::weak_ptr<TComponent> get_component() const
    {
        auto it = m_component_store.find(std::type_index(typeid(TComponent)));
        if (it != m_component_store.end())
        {
            return std::static_pointer_cast<TComponent>(it->second);
        }
        return std::weak_ptr<TComponent>();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Non-template overload of get_component, used internally by Component to avoid a
    ///        circular header dependency. Prefer get_component<T>() at all other call sites.
    ///
    /// @param [in] type - The std::type_index of the desired Component type.
    ///
    /// @return A std::weak_ptr<Component> to the Component, or an empty std:::weak_ptr<TComponent>
    ///         if not found.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    std::weak_ptr<Component> get_component_by_type(std::type_index type) const;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Searches for a component of the given TComponent type. Optionally searches through
    ///        the pending components as well.
    ///
    /// @tparam TComponent Templated Component type to search for. Requires that this type is
    ///                    derived from Component.
    ///
    /// @param [in] search_pending - Optional parameter to additionally search through the pending
    ///                              Components. If true, has_component<> will search both the
    ///                              pending Components and the active Components for TComponent. If
    ///                              false, has_component<> will only search the active Components.
    ///                              Defaults to true.
    ///
    /// @return Returns true if there is a TComponent on this Entity.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename TComponent>
    requires(std::derived_from<TComponent, Component>)
    bool has_component(bool search_pending = true) const
    {
        std::type_index const type{ typeid(TComponent) };

        // Check pending components. O(n), but this list is typically tiny
        bool const is_pending = std::ranges::any_of(
            m_pending_components,
            [&type](std::shared_ptr<Component> const& pending)
            {
                return std::type_index{ typeid(*pending) } == type;
            });

        // Check already-initialized components. O(1)
        bool const active = m_component_store.contains(type);

        // (is_pending or active) if search_pending, else (active) 
        return search_pending ? (is_pending || active) : active;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return Returns this Entity's update order.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    std::uint8_t get_update_order() const { return m_update_order; }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return Returns this Entity's current state.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    EState get_entity_state() const { return m_state; }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return Gets the Layer that owns this Entity.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    Layer& get_owning_layer() const { return m_owning_layer; }

private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Enum that allows insert_component_sorted() to gather the correct sorting order from
    ///        Components during insertion.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    enum EComponentInsertType
    {
        Update,
        Render,
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Flushes all pending Components into the active component store, then calls
    ///        Component::awake() on each newly-added Component. Two-pass: all components reach
    ///        the active store before any awake_component() override runs, so siblings are always
    ///        accessible via get_sibling_component<T>(). Not overridable.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void awake_components();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Calls Component::start() on every active Component. Called from Entity::start()
    ///        and for runtime-added Components once the Entity is Active. Not overridable.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void start_components();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Updates all the Components attached to the Entity. Called from Entity::update(). Not
    ///        overridable.
    ///
    /// @param [in] delta_time - Time since last update. 
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void update_components(float delta_time);
    void late_update_components(float delta_time);
    void fixed_update_components(float fixed_dt);
    void render_components(SDL_Renderer* renderer);
    void cleanup_components();
    void on_component_added() const;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Inserts a Component into one of the Component collections based on a given sorting
    ///        order.
    ///
    /// @param [in] component_collection - Collection of Components that the Component will be
    ///                                    inserted into.
    /// @param [in] component            - Component that will be inserted into the collection.
    /// @param [in] insert_sorter        - The sorting order demarcation for the Component.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void insert_component_sorted(std::vector<std::weak_ptr<Component>>& component_collection,
                                 std::weak_ptr<Component> component,
                                 EComponentInsertType insert_sorter);
    
    Layer& m_owning_layer;
    std::uint8_t m_update_order;
    EState m_state{ EState::Pending };

    std::vector<std::shared_ptr<Component>> m_pending_components{};
    std::unordered_map<std::type_index, std::shared_ptr<Component>> m_component_store{};
    std::vector<std::weak_ptr<Component>> m_update_ordered_components{};
    std::vector<std::weak_ptr<Component>> m_render_ordered_components{};
};

} // namespace Core

#endif // CORE_ENTITY_H
