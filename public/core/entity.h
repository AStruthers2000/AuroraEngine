////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Base Entity class. Root of all objects in a Layer. Override initialize_entity(),
///        update_entity(), and/or render_entity() for custom behavior.
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ENGINE_ENTITY_H
#define ENGINE_ENTITY_H

#include "core/component.h"
#include "core/layer.h"
#include "core/components/transform_component.h"

#include <SDL3/SDL.h>

#include <cstdint>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Core
{

class Entity
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief State of the Entity. Controls how the Entity is updated.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    enum class EState
    {
        Active,     ///< Entity is in the "normal" state; Entity::update() is called every tick.
        Inactive,   ///< Entity is paused. Entity::update() will not be called, but the Entity can
                    ///< return to the Active state with no overhead.
        Pending,    ///< Entity has been constructed but not yet awoken. On the next initialization
                    ///< pass, Entity::awake() will be called, transitioning to Awoken.
        Awoken,     ///< Entity has completed awake() but has not yet started. All Components are
                    ///< initialized and accessible. Entity::start() will be called once all sibling
                    ///< Entities in the owning Layer have also awoken.
        Destroyed,  ///< Entity has been marked for destruction. Entity::~Entity() will be called on
                    ///< the next update cycle
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Entity constructor.
    ///
    /// @param [in] owning_world - Entity must know what world owns it. 
    ////////////////////////////////////////////////////////////////////////////////////////////////
    Entity(Layer& owning_world, std::uint8_t update_order = DEFAULT_SORTING_ORDER);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Entity destructor.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~Entity() = default;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Phase 1 of Entity initialization. Flushes all pending Components into the Component
    ///        store and calls awake_entity(). Transitions state from Pending to Awoken. Not
    ///        overridable.
    ///
    /// @note Called by the owning Layer before start(). Guaranteed to be called before any Entity
    ///       in the Layer calls start(), which means awake_entity() overrides must not assume other
    ///       Entities have awoken yet.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void awake();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Runs any Entity-specific Phase 1 initialization code. Called from Entity::awake().
    ///        Overridable.
    ///
    /// @note Contract: all Components added during construction are accessible via
    ///       get_component<T>(). Do not access Components or state on other Entities here; their
    ///       awake() is not guaranteed to have run yet.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void awake_entity();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Phase 2 of Entity initialization. Calls start_entity() and transitions state from
    ///        Awoken to Active. Not overridable.
    ///
    /// @note Called by the owning Layer after every pending Entity in the same initialization pass
    ///       has completed awake(). It is safe to access other Entities and their Components here.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void start();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Runs any Entity-specific Phase 2 initialization code. Called from Entity::start().
    ///        Overridable.
    ///
    /// @note Contract: all Entities that were pending in the same initialization pass have
    ///       completed awake_entity(), so their Components are fully initialized and accessible.
    ///       Use this override, rather than awake_entity(), for any setup that requires references
    ///        to other Entities or their Components.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void start_entity();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Updates this Entity. Called from owning Layer. Not overridable.
    ///
    /// @param [in] delta_time - Time since last update. 
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void update(float delta_time);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Runs any Entity-specific update code. Called from Entity::update(). Overridable.
    ///
    /// @param [in] delta_time - Time since last update. 
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void update_entity(float delta_time);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Renders this Entity. Called from owning Layer. Not overridable.
    ///
    /// @param [in] renderer - Renderer provided by the owning Layer. 
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void render(SDL_Renderer* renderer);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Runs any Entity-specific render code. Called from Entity::render(). Overridable.
    ///
    /// @param [in] renderer - Renderer provided by the owning Layer. 
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void render_entity(SDL_Renderer* renderer);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Adds a Component to this Entity. Entity assumes ownership of the Component.
    ///
    /// @param [in] component - Component to be added to this Entity. 
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void add_component(std::unique_ptr<Component> component);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief 
    /// @param component 
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // void remove_component(std::unique_ptr<Component> component);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Marks this Entity for destruction. Destruction occurs at the end of this frame.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void destroy_entity();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return Returns this Entity's update order.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    std::uint8_t get_update_order() const { return m_update_order; }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return Returns this Entity's current state.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    EState get_entity_state() const { return m_state; }

protected:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Tries to get the component of type TComponent owned by this Entity. Uses RTTI to
    ///        perform a O(1) lookup.
    ///
    /// @tparam TComponent Templated Component type to search for. Requires that this type is
    ///                    derived from Component.
    ///
    /// @return Returns a raw pointer to the Component of type TComponent, or nullptr if no
    ///         Component of that type was found.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename TComponent>
    requires(std::is_base_of_v<Component, TComponent>)
    TComponent* get_component() const
    {
        auto it = m_component_store.find(std::type_index(typeid(TComponent)));
        if (it != m_component_store.end())
        {
            return static_cast<TComponent*>(it->second.get());
        }
        return nullptr;
    }

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
    /// @brief Initializes all the Components attached to this Entity. Called from
    ///        Entity::initialize(). Not overridable. 
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void initialize_components();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Updates all the Components attached to the Entity. Called from Entity::update(). Not
    ///        overridable.
    ///
    /// @param [in] delta_time - Time since last update. 
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void update_components(float delta_time);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Renders all the Components attached to the Entity. Called from Entity::render(). Not
    ///        overridable.
    ///
    /// @param [in] renderer - Renderer provided by the owning Layer. 
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void render_components(SDL_Renderer* renderer);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Inserts a Component into one of the Component collections based on a given sorting
    ///        order.
    ///
    /// @param [in] component_collection - Collection of Components that the Component will be
    ///                                    inserted into.
    /// @param [in] component            - Component that will be inserted into the collection.
    /// @param [in] insert_sorter        - The sorting order demarcation for the Component.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void insert_component_sorted(std::vector<Component*>& component_collection,
                                 Component* component,
                                 EComponentInsertType insert_sorter);
    
    Layer& m_world;
    std::uint8_t m_update_order;
    EState m_state{ EState::Pending };

    std::vector<std::unique_ptr<Component>> m_pending_components{};
    std::unordered_map<std::type_index, std::unique_ptr<Component>> m_component_store{};
    std::vector<Component*> m_update_ordered_components{};
    std::vector<Component*> m_render_ordered_components{};
};

} // namespace Core

#endif // ENGINE_ENTITY_H
