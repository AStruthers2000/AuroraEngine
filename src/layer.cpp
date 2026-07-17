////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "core/layer.h"

#include "core/engine.h"
#include "core/entity.h"

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
void Layer::initialize()
{
    initialize_entities();
    initialize_layer();
}

//--------------------------------------------------------------------------------------------------
void Layer::initialize_layer()
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
            entity->awake();
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
            entity->start();
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
void Layer::update(float delta_time)
{
    update_entities(delta_time);
    update_layer(delta_time);
}

//--------------------------------------------------------------------------------------------------
void Layer::update_layer(float delta_time)
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
        entity->update(delta_time);
    }

    // Release all entities marked for destruction
    std::vector<std::shared_ptr<Entity>> destroyed_entities{};
    for (auto const& entity : m_entities)
    {
        if (entity->get_entity_state() == Entity::EState::Destroyed)
        {
            destroyed_entities.emplace_back(std::move(entity));
        }
    }

    for (auto& entity : destroyed_entities)
    {
        entity.reset();
    }

    m_entities.erase(
        std::remove_if(m_entities.begin(), m_entities.end(),
            [](auto const& entity)
            {
                return entity->get_entity_state() == Entity::EState::Destroyed;
            }),
        m_entities.end());
}

//--------------------------------------------------------------------------------------------------
void Layer::render(SDL_Renderer* renderer)
{
    render_entities(renderer);
    render_layer(renderer);
}

//--------------------------------------------------------------------------------------------------
void Layer::render_layer(SDL_Renderer* renderer)
{
    // Intentionally left blank; virtual function.
}

//--------------------------------------------------------------------------------------------------
void Layer::render_entities(SDL_Renderer* renderer)
{
    for (auto const& entity : m_entities)
    {
        entity->render(renderer);
    }
}

//--------------------------------------------------------------------------------------------------
void Layer::cleanup()
{
    cleanup_entities();
    cleanup_layer();
}

//--------------------------------------------------------------------------------------------------
void Layer::cleanup_layer()
{
    // Intentionally left blank; virtual function.
}

//--------------------------------------------------------------------------------------------------
void Layer::cleanup_entities()
{
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

} // namespace Core
