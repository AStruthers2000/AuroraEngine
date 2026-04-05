#include "engine/world.h"

#include "engine/engine.h"
#include "engine/game_object.h"

#include <algorithm>

namespace AuroraEngine
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
    // Move all pending objects to m_game_objects
    for (auto& [priority, game_object] : m_pending_initialize_objects)
    {
        game_object->set_object_state(GameObject::EGameObjectState::Active);
        insert_sorted(priority, std::move(game_object), m_game_objects);
    }
    m_pending_initialize_objects.clear();

    // Initialize all game objects
    for (auto& [_, game_object] : m_game_objects)
    {
        game_object->initialize();
    }
}

//--------------------------------------------------------------------------------------------------
void GameWorld::cleanup()
{
    for (auto& [_, game_object] : m_game_objects)
    {
        if (game_object)
        {
            game_object->cleanup();
        }
    }
}

//--------------------------------------------------------------------------------------------------
bool GameWorld::add_object(std::shared_ptr<GameObject> object, int update_order)
{
    object->set_object_state(GameObject::EGameObjectState::Pending);
    insert_sorted(update_order, std::move(object), m_pending_initialize_objects);
    return true;
}

//--------------------------------------------------------------------------------------------------
void GameWorld::update(float delta_time)
{
    // Initialize all pending objects and move to m_game_objects.
    if (!m_pending_initialize_objects.empty())
    {
        for (auto &[priority, game_object]: m_pending_initialize_objects)
        {
            game_object->set_object_state(GameObject::EGameObjectState::Active);
            game_object->initialize();
            insert_sorted(priority, std::move(game_object), m_game_objects);
        }
        m_pending_initialize_objects.clear();
    }

    // Update all active objects, and clean up / erase destroyed objects in a single pass.
    auto it = m_game_objects.begin();
    while (it != m_game_objects.end())
    {
        auto& [priority, game_object] = *it;

        if (game_object->get_object_state() == GameObject::EGameObjectState::Active)
        {
            game_object->update(delta_time);
        }

        // Check state again — update() may have marked the object as Destroyed.
        if (game_object->get_object_state() == GameObject::EGameObjectState::Destroyed)
        {
            game_object.reset();
            it = m_game_objects.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

//--------------------------------------------------------------------------------------------------
void GameWorld::render(SDL_Renderer* renderer)
{
    for (auto& [_, game_object] : m_game_objects)
    {
        if (game_object->get_object_state() == GameObject::EGameObjectState::Active)
        {
            game_object->render(renderer);
        }
    }
}

void GameWorld::insert_sorted(int priority, std::shared_ptr<GameObject> object, std::list<PrioritizedObject>& collection)
{
    // Find the first element with a higher update order
    auto it = std::find_if(collection.begin(), collection.end(),
                           [priority](const auto &pair) {
                               return pair.first > priority;
                           });

    // Insert before that element (or at the end if not found)
    collection.insert(it, std::make_pair(priority, std::move(object)));
}

} // namespace AuroraEngine
