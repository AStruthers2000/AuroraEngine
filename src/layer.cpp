////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "core/layer.h"

#include "core/engine.h"
#include "core/entity.h"
#include "core/events/event.h"

#include <algorithm>
#include <print>

namespace Core
{

//--------------------------------------------------------------------------------------------------
Layer::Layer(Engine& owning_engine)
    : m_engine(owning_engine)
{
}

//--------------------------------------------------------------------------------------------------
Layer::~Layer() = default;

//--------------------------------------------------------------------------------------------------
void Layer::broadcast_event(Event& event)
{
    Engine::get().broadcast_event(event);
}

//--------------------------------------------------------------------------------------------------
void Layer::propagate_event_down(Event& event)
{
    // Broadcast Event to all owned Entities
    for (auto& entity : m_entities)
    {
        entity->propagate_event_down(event);
        if (event.get_handled())
        {
            break;
        }
    }

    // If the Event wasn't handled by one of the Entities, allow the Layer to handle the Event
    if (!event.get_handled())
    {
        on_event(event);
    }
}

//--------------------------------------------------------------------------------------------------
void Layer::broadcast_event_within_layer(Event& event)
{
    propagate_event_down(event);
}

//--------------------------------------------------------------------------------------------------
void Layer::on_event(Event& event)
{
    // Intentionally left blank; virtual function.
}

//--------------------------------------------------------------------------------------------------
void Layer::initialize_layer()
{
    initialize_entities();
    initialize();
}

//--------------------------------------------------------------------------------------------------
void Layer::initialize()
{
    // Intentionally left blank; virtual function.
}

//--------------------------------------------------------------------------------------------------
void Layer::initialize_entities()
{
    for (auto const& entity : m_pending_entities)
    {
        if (entity->get_entity_state() == Entity::EState::Pending)
        {
            entity->awake_entity();
        }
        else
        {
            std::println("[Layer::initialize_entities] An Entity in the pending list couldn't be "
                         "awoken, because its State was: %d",
                         static_cast<int>(entity->get_entity_state()));
        }
    }

    for (auto const& entity : m_pending_entities)
    {
        if (entity->get_entity_state() == Entity::EState::Awoken)
        {
            entity->start_entity();
            move_entity_to_active(entity);
        }
        else
        {
            std::println("[Layer::initialize_entities] An Entity in the pending list couldn't be "
                         "started, because its State was: %d",
                         static_cast<int>(entity->get_entity_state()));
        }
    }
    m_pending_entities.clear();
}

//--------------------------------------------------------------------------------------------------
void Layer::update_layer(float delta_time)
{
    if (m_paused) return;

    update(delta_time);
    update_entities(delta_time);
    late_update(delta_time);
}

//--------------------------------------------------------------------------------------------------
void Layer::update(float delta_time)
{
    // Intentionally left blank; virtual function.
}

//--------------------------------------------------------------------------------------------------
void Layer::late_update(float delta_time)
{
    // Intentionally left blank; virtual function.
}

//--------------------------------------------------------------------------------------------------
void Layer::update_entities(float delta_time)
{
    // Initialize and move all pending entities
    initialize_entities();

    // Update all entities
    for (auto const& entity : m_entities)
    {
        entity->update_entity(delta_time);
    }

    // Release all entities marked for destruction (calls cleanup before destroying)
    for (auto it = m_entities.begin(); it != m_entities.end(); )
    {
        if ((*it)->get_entity_state() == Entity::EState::Destroyed)
        {
            (*it)->cleanup_entity();
            it = m_entities.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

//--------------------------------------------------------------------------------------------------
void Layer::render_layer(SDL_Renderer* renderer)
{
    render_entities(renderer);
    render(renderer);
}

//--------------------------------------------------------------------------------------------------
void Layer::render(SDL_Renderer* renderer)
{
    // Intentionally left blank; virtual function.
}

//--------------------------------------------------------------------------------------------------
void Layer::render_entities(SDL_Renderer* renderer)
{
    for (auto const& entity : m_entities)
    {
        entity->render_entity(renderer);
    }
}

//--------------------------------------------------------------------------------------------------
void Layer::cleanup_layer()
{
    cleanup_entities();
    cleanup();
}

//--------------------------------------------------------------------------------------------------
void Layer::cleanup()
{
    // Intentionally left blank; virtual function.
}

//--------------------------------------------------------------------------------------------------
void Layer::cleanup_entities()
{
    for (auto const& entity : m_entities)
    {
        entity->cleanup_entity();
    }
    m_pending_entities.clear();
    m_entities.clear();
}

//--------------------------------------------------------------------------------------------------
void Layer::move_entity_to_active(std::shared_ptr<Entity> entity)
{
    std::uint8_t priority = entity->get_update_order();

    // Find the first element with a higher update order
    auto it = std::find_if(
        m_entities.begin(),
        m_entities.end(),
        [priority](std::shared_ptr<Entity> const& other)
        {
            return other->get_update_order() > priority;
        });

    // Insert before that element (or at the end if not found)
    m_entities.insert(it, std::move(entity));
}

//--------------------------------------------------------------------------------------------------
void Layer::fixed_update_layer(float fixed_dt)
{
    if (m_paused) return;
    fixed_update_entities(fixed_dt);
    fixed_update(fixed_dt);
}

//--------------------------------------------------------------------------------------------------
void Layer::fixed_update(float fixed_dt)
{
    // Intentionally left blank; virtual function.
}

//--------------------------------------------------------------------------------------------------
void Layer::fixed_update_entities(float fixed_dt)
{
    for (auto const& entity : m_entities)
    {
        entity->fixed_update_entity(fixed_dt);
    }
}

//--------------------------------------------------------------------------------------------------
void Layer::on_entity_added() const
{
    assert(!Engine::get().is_rendering());
    assert(!Engine::get().is_cleaning_up());
}

} // namespace Core
