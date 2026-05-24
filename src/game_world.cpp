////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "core/game_world.h"

#include "core/engine.h"
#include "core/entity.h"

#include <algorithm>
#include <print>

namespace Core
{

//--------------------------------------------------------------------------------------------------
GameWorld::GameWorld(Engine& owning_engine)
    : m_engine(owning_engine)
{
}

//--------------------------------------------------------------------------------------------------
GameWorld::~GameWorld() = default;

//--------------------------------------------------------------------------------------------------
void GameWorld::initialize()
{
    initialize_entities();
    initialize_world();
}

//--------------------------------------------------------------------------------------------------
void GameWorld::initialize_world()
{
    // Intentionally left blank; virtual function.
}

//--------------------------------------------------------------------------------------------------
void GameWorld::initialize_entities()
{
    for (auto const& entity : m_pending_entities)
    {
        if (entity->get_entity_state() == Entity::EState::Pending)
        {
            entity->initialize();
            move_entity_to_active(entity);
        }
        else
        {
            std::println("[GameWorld::initialize_entities] An Entity was in the pending list but "
                         "its State was: %d",
                         static_cast<int>(entity->get_entity_state()));
        }
    }
    m_pending_entities.clear();
}

//--------------------------------------------------------------------------------------------------
void GameWorld::update(float delta_time)
{
    update_entities(delta_time);
    update_world(delta_time);
}

//--------------------------------------------------------------------------------------------------
void GameWorld::update_world(float delta_time)
{
    // Intentionally left blank; virtual function.
}

//--------------------------------------------------------------------------------------------------
void GameWorld::update_entities(float delta_time)
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
}

//--------------------------------------------------------------------------------------------------
void GameWorld::render(SDL_Renderer* renderer)
{
    render_entities(renderer);
    render_world(renderer);
}

//--------------------------------------------------------------------------------------------------
void GameWorld::render_world(SDL_Renderer* renderer)
{
    // Intentionally left blank; virtual function.
}

//--------------------------------------------------------------------------------------------------
void GameWorld::render_entities(SDL_Renderer* renderer)
{
    for (auto const& entity : m_entities)
    {
        entity->render(renderer);
    }
}

//--------------------------------------------------------------------------------------------------
void GameWorld::cleanup()
{
    cleanup_entities();
    cleanup_world();
}

//--------------------------------------------------------------------------------------------------
void GameWorld::cleanup_world()
{
    // Intentionally left blank; virtual function.
}

//--------------------------------------------------------------------------------------------------
void GameWorld::cleanup_entities()
{
    while (!m_pending_entities.empty())
    {
        m_pending_entities.back().reset();
    }

    while (!m_entities.empty())
    {
        m_entities.back().reset();
    }
}

//--------------------------------------------------------------------------------------------------
void GameWorld::add_entity(std::shared_ptr<Entity> entity)
{
    m_pending_entities.emplace_back(std::move(entity));
}

//--------------------------------------------------------------------------------------------------
void GameWorld::move_entity_to_active(std::shared_ptr<Entity> entity)
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
