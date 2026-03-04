////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ENGINE_GAMEWORLD_H
#define ENGINE_GAMEWORLD_H

#include <SDL3/SDL.h>

#include <list>
#include <memory>
#include <utility>

namespace AuroraEngine
{

class Engine;
class GameObject;

class GameWorld
{
public:
    explicit GameWorld(Engine& owning_engine);
    ~GameWorld();

    Engine& get_engine() { return m_engine; }

    /// @brief Adds an object to the world.
    /// @note initialize() and update() may call add_object(), which inserts into
    /// m_pending_initialize_objects, not m_game_objects, so iterators on m_game_objects remain
    /// valid. Newly added objects will be initialized next frame.
    bool add_object(std::unique_ptr<GameObject> object, int update_order);

    void initialize();
    void update(float delta_time);
    void render(SDL_Renderer* renderer);
    void cleanup();

private:
    Engine& m_engine;

    using PrioritizedObject = std::pair<int, std::unique_ptr<GameObject>>;
    static void insert_sorted(int priority, std::unique_ptr<GameObject> object, std::list<PrioritizedObject>& collection);

    std::list<PrioritizedObject> m_game_objects;

    std::list<PrioritizedObject> m_pending_initialize_objects;
};

} // namespace AuroraEngine

#endif // ENGINE_GAMEWORLD_H
