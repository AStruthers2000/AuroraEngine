////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "core/entity.h"

#include <print>

namespace Core
{

//--------------------------------------------------------------------------------------------------
Entity::Entity(GameWorld& owning_world, std::uint8_t update_order)
    : m_world(owning_world)
    , m_transform(*this)
{
}

//--------------------------------------------------------------------------------------------------
void Entity::initialize()
{
    if (m_state == EState::Pending)
    {
        initialize_components();
        initialize_entity();
        m_state = EState::Active;
    }
    else
    {
        std::println("[Entity::initialize] initialize() was called more than once.");
    }
}

//--------------------------------------------------------------------------------------------------
void Entity::initialize_entity()
{
    // Intentionally left blank; virtual function.
}

//--------------------------------------------------------------------------------------------------
void Entity::initialize_components()
{
    for (auto const& component : m_component_store)
    {
        component->initialize();
    }
}

//--------------------------------------------------------------------------------------------------
void Entity::update(float delta_time)
{
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
    for (Component* component : m_update_ordered_components)
    {
        component->update_component(delta_time);
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
    for (Component* component : m_render_ordered_components)
    {
        component->render_component(renderer);
    }
}

//--------------------------------------------------------------------------------------------------
void Entity::add_component(std::unique_ptr<Component> component)
{
    Component* raw = component.get();
    m_component_store.emplace(std::move(component));

    insert_component_sorted(m_update_ordered_components, raw, EComponentInsertType::Update);
    insert_component_sorted(m_render_ordered_components, raw, EComponentInsertType::Render);
}

//--------------------------------------------------------------------------------------------------
void Entity::destroy_entity()
{
    m_state = EState::Destroyed;
}

//--------------------------------------------------------------------------------------------------
void Entity::insert_component_sorted(std::vector<Component*>& component_collection,
                                     Component* component,
                                     EComponentInsertType insert_sorter)
{
    auto get_priority = [insert_sorter](Component const* c) -> std::uint8_t
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

    std::uint8_t const my_priority = get_priority(component);

    // Find the first component with a higher order
    auto it = std::find_if(
        component_collection.begin(),
        component_collection.end(),
        [my_priority, get_priority](Component const* other)
        {
            return get_priority(other) > my_priority;
        });

    component_collection.insert(it, component);
}

} // namespace Core
