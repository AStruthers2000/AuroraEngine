////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ENGINE_ENGINE_H
#define ENGINE_ENGINE_H

// #include "core/window.h"
// #include "core/subsystems/input/input_subsystem.h"

#include <memory>
// #include <string_view>

namespace Core
{

class GameWorld;

class Engine
{
public:
    Engine();
    // explicit Engine(WindowSpecification const& window_spec);
    ~Engine();

    void initialize(std::unique_ptr<GameWorld> managed_world);
    void run();

    // InputSubsystem& get_input_subsystem() { return m_input_subsystem; }

    // static Engine& get();
    // Window& get_window() const { return *m_window; }

private:
    bool process_input();
    void update(float delta_time);
    void render();

    bool m_initialized = false;
    // std::unique_ptr<Window> m_window;
    std::unique_ptr<GameWorld> m_managed_world;
    // InputSubsystem m_input_subsystem;
};

} // namespace Core

#endif // ENGINE_ENGINE_H
