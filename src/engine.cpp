////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "core/engine.h"

#include "window.h"
#include "core/layer.h"

#include <glm/glm.hpp>

namespace Core
{

static Engine* s_engine = nullptr;

//--------------------------------------------------------------------------------------------------
Engine::Engine(WindowSpecification const& window_spec)
{
    if (s_engine)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                 "Engine Initialization",
                                 "Multiple instances of Engine detected. There should only ever be "
                                 "one instance of Engine.",
                                 nullptr);
        assert(false);
        std::exit(-1);
    }

    // Initialize SDL
    if (!SDL_Init(window_spec.sdl_init_flags))
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                 "Error",
                                 "Failed to initialize SDL3!",
                                 nullptr);
        std::exit(-1);
    }

    // Initialize window
    m_window = std::make_unique<Window>(window_spec);
    m_window->create();

    // Set static ptr to this engine
    s_engine = this;
}

//--------------------------------------------------------------------------------------------------
Engine::~Engine()
{
    // Cleanup all pending layers
    for (auto const& pending : m_pending_layers)
    {
        pending->cleanup();
    }
    m_pending_layers.clear();

    // Cleanup all active layers
    for (auto const& layer : m_layer_stack)
    {
        layer->cleanup();
    }
    m_layer_stack.clear();

    // Cleanup window
    m_window->destroy();
    m_window.reset();

    // Quit SDL
    SDL_Quit();

    // Reset static engine ptr
    s_engine = nullptr;
}

//--------------------------------------------------------------------------------------------------
void Engine::run()
{
    m_running = true;

    // Using a variable time step method
    std::uint64_t previous_frame_time = SDL_GetTicksNS();
    while (m_running)
    {
        std::uint64_t frame_start_time = SDL_GetTicksNS();
        float delta_time = static_cast<float>(frame_start_time - previous_frame_time) / 1E+09;
        delta_time = glm::clamp(delta_time, 0.001f, 0.1f);

        initialize_pending_layers();

        bool exit_indicator = process_input();
        if (exit_indicator)
        {
            stop();
            break;
        }

        update(delta_time);
        render();

        previous_frame_time = frame_start_time;
    }
}

//--------------------------------------------------------------------------------------------------
void Engine::stop()
{
    m_running = false;
}

//--------------------------------------------------------------------------------------------------
Engine& Engine::get()
{
    assert(s_engine);
    return *s_engine;
}

//--------------------------------------------------------------------------------------------------
void Engine::initialize_pending_layers()
{
    // Initialize and move all pending worlds
    for (auto& layer : m_pending_layers)
    {
        layer->initialize();
        m_layer_stack.push_back(std::move(layer));
    }
    m_pending_layers.clear();
}

//--------------------------------------------------------------------------------------------------
bool Engine::process_input()
{
    // return m_input_subsystem.test();

    bool quit_requested{ false };

    SDL_Event event{ 0 };
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_EventType::SDL_EVENT_QUIT:
                quit_requested = true;
                break;
            // case SDL_EventType::SDL_EVENT_WINDOW_RESIZED:
            //     Engine::get().get_window().set_size({event.window.data1, event.window.data2});
            //     break;
        }
    }

    return quit_requested;
}

//--------------------------------------------------------------------------------------------------
void Engine::update(float delta_time)
{
    for (auto const& layer : m_layer_stack)
    {
        layer->update(delta_time);
    }
}

//--------------------------------------------------------------------------------------------------
void Engine::render()
{
    SDL_Renderer* renderer = m_window->get_sdl_renderer();
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Perform all rendering
    for (auto const& layer : m_layer_stack)
    {
        layer->render(renderer);
    }

    // Swap buffers and present
    SDL_RenderPresent(renderer);
}

//--------------------------------------------------------------------------------------------------
glm::vec2 Engine::get_window_size() const
{
    SDL_Window* window = get_window().get_sdl_window();

    int x{ 0 }; int y{ 0 };
    SDL_GetWindowSizeInPixels(window, &x, &y);

    return glm::vec2{x, y};
}

} // namespace Core
