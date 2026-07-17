////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "core/entity.h"

namespace Core
{

//--------------------------------------------------------------------------------------------------
Entity::Entity(Layer& owning_layer, std::uint8_t update_order)
    : m_owning_layer(owning_layer)
{
}

//--------------------------------------------------------------------------------------------------
void Entity::awake()
{
    if (m_state == EState::Pending)
    {
        // Try to add a TransformComponent right before initializing components. This allows the
        // user to add their own transform component during construction if they'd like, but
        // guarantees that the Entity will have a transform component before awake_entity().
        if (!has_component<TransformComponent>())
        {
            add_component<TransformComponent>();
        }

        awake_components();
        awake_entity();
        m_state = EState::Awoken;
    }
    else
    {
        std::println("[Entity::awake] awake() called in unexpected state: %d",
                     static_cast<int>(m_state));
    }
}

//--------------------------------------------------------------------------------------------------
void Entity::awake_entity()
{
    // Intentionally left blank; virtual function.
}

//--------------------------------------------------------------------------------------------------
void Entity::awake_components()
{
    // Snapshot the pending list so we can track which components are newly added
    std::vector<std::shared_ptr<Component>> newly_added{};
    newly_added.swap(m_pending_components);

    // Pass 1: flush ALL pending components into the active store before calling any virtual code.
    // This guarantees every sibling is reachable via get_component<T>() during awake_component().
    for (std::shared_ptr<Component> const& component : newly_added)
    {
        insert_component_sorted(m_update_ordered_components, component, EComponentInsertType::Update);
        insert_component_sorted(m_render_ordered_components, component, EComponentInsertType::Render);
        m_component_store.try_emplace(std::type_index(typeid(*component.get())), component);
    }

    // Pass 2: awake each newly-added component now that all siblings are in the active store.
    for (std::shared_ptr<Component> const& component : newly_added)
    {
        component->awake();
    }

    // If this entity is already active (runtime component addition), immediately start them too.
    if (m_state == EState::Active)
    {
        for (std::shared_ptr<Component> const& component : newly_added)
        {
            component->start();
        }
    }
}

//--------------------------------------------------------------------------------------------------
void Entity::start()
{
    if (m_state == EState::Awoken)
    {
        start_components();
        start_entity();
        m_state = EState::Active;
    }
    else
    {
        std::println("[Entity::start] start() called in unexpected state: %d",
                     static_cast<int>(m_state));
    }
}

//--------------------------------------------------------------------------------------------------
void Entity::start_entity()
{
    // Intentionally left blank; virtual function.
}

//--------------------------------------------------------------------------------------------------
void Entity::start_components()
{
    for (std::weak_ptr<Component> const& component : m_update_ordered_components)
    {
        if (auto component_ptr = component.lock())
        {
            component_ptr->start();
        }
    }
}

//--------------------------------------------------------------------------------------------------
void Entity::update(float delta_time)
{
    awake_components();
    update_components(delta_time);
    update_entity(delta_time);
}

//--------------------------------------------------------------------------------------------------
void Entity::update_entity(float delta_time)
{
    // Intentionally left blank; virtual function.
}

//--------------------------------------------------------------------------------------------------
void Entity::update_components(float delta_time)
{
    for (std::weak_ptr<Component> component : m_update_ordered_components)
    {
        if (auto component_ptr = component.lock())
        {
            component_ptr->update_component(delta_time);
        }
    }
}

//--------------------------------------------------------------------------------------------------
void Entity::render(SDL_Renderer* renderer)
{
    render_components(renderer);
    render_entity(renderer);
}

//--------------------------------------------------------------------------------------------------
void Entity::render_entity(SDL_Renderer* renderer)
{
    // Intentionally left blank; virtual function.
}

//--------------------------------------------------------------------------------------------------
void Entity::render_components(SDL_Renderer* renderer)
{
    for (std::weak_ptr<Component> component : m_render_ordered_components)
    {
        if (auto component_ptr = component.lock())
        {
            component_ptr->render_component(renderer);
        }
    }
}

//--------------------------------------------------------------------------------------------------
void Entity::destroy_entity()
{
    m_state = EState::Destroyed;
}

//--------------------------------------------------------------------------------------------------
std::weak_ptr<Component> Entity::get_component_by_type(std::type_index type) const
{
    auto it = m_component_store.find(type);
    if (it != m_component_store.end())
    {
        return it->second;
    }
    return std::weak_ptr<Component>();
}

//--------------------------------------------------------------------------------------------------
void Entity::insert_component_sorted(std::vector<std::weak_ptr<Component>>& component_collection,
                                     std::weak_ptr<Component> component,
                                     EComponentInsertType insert_sorter)
{
    auto get_priority = [insert_sorter](std::shared_ptr<Component> const c) -> std::uint8_t
    {
        std::uint8_t priority = 0;

        switch (insert_sorter)
        {
            case EComponentInsertType::Update:
                priority = c->get_update_order();
                break;
            case EComponentInsertType::Render:
                priority = c->get_render_order();
                break;
            default:
                printf("[Entity::insert_component_sorted] Component insert type not recognized: "
                       "%d\n",
                       static_cast<int>(insert_sorter));
                break;
        }

        return priority;
    };

    std::uint8_t const my_priority = get_priority(component.lock());

    // Find the first component with a higher order
    auto it = std::find_if(
        component_collection.begin(),
        component_collection.end(),
        [my_priority, get_priority](std::weak_ptr<Component> other)
        {
            if (auto shared = other.lock())
            {
                return get_priority(shared) > my_priority;
            }
            return false;
        });

    component_collection.insert(it, component);
}

} // namespace Core
