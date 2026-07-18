////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "core/engine.h"

#include "window.h"
#include "core/layer.h"
#include "core/events/events_public.h"

#include <glm/glm.hpp>

#include <ranges>

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
    m_is_cleaning_up = true;

    // Cleanup all pending layers
    for (auto const& pending : m_pending_layers)
    {
        pending->cleanup_layer();
    }
    m_pending_layers.clear();

    // Cleanup all active layers
    for (auto const& layer : m_layer_stack)
    {
        layer->cleanup_layer();
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

    // Using a variable time step with semi-fixed accumulator for physics
    std::uint64_t previous_frame_time = SDL_GetTicksNS();
    while (m_running)
    {
        std::uint64_t frame_start_time = SDL_GetTicksNS();
        float delta_time = static_cast<float>(frame_start_time - previous_frame_time) / 1E+09;
        delta_time = glm::clamp(delta_time, 0.001f, 0.1f);

        apply_pending_transitions();
        initialize_pending_layers();

        bool exit_indicator = process_input();
        if (exit_indicator)
        {
            stop();
            break;
        }

        m_accumulator += delta_time;
        m_is_updating = true;
        while (m_accumulator >= m_fixed_step)
        {
            fixed_update(m_fixed_step);
            m_accumulator -= m_fixed_step;
        }
        update(delta_time);
        m_is_updating = false;

        m_is_rendering = true;
        render();
        m_is_rendering = false;

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
void Engine::broadcast_event(Event& event)
{
    for (auto& layer : std::views::reverse(m_layer_stack))
    {
        layer->on_event(event);
        if (event.get_handled())
        {
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
void Engine::initialize_pending_layers()
{
    // Initialize and move all pending worlds
    for (auto& layer : m_pending_layers)
    {
        layer->initialize_layer();
        m_layer_stack.push_back(std::move(layer));
    }
    m_pending_layers.clear();
}

//--------------------------------------------------------------------------------------------------
bool Engine::process_input()
{
    // return m_input_subsystem.test();

    bool quit_requested{ false };

    SDL_Event sdl_event{ 0 };
    while (SDL_PollEvent(&sdl_event))
    {
        switch (sdl_event.type)
        {
            case SDL_EventType::SDL_EVENT_QUIT:
            {
                quit_requested = true;

                WindowCloseEvent event;
                broadcast_event(event);
                break;
            }
            case SDL_EventType::SDL_EVENT_WINDOW_RESIZED:
            {
                WindowResizeEvent event(sdl_event.window.data1, sdl_event.window.data2);
                broadcast_event(event);
                break;
            }
            case SDL_EventType::SDL_EVENT_KEY_DOWN:
            {
                KeyPressedEvent event(sdl_event.key.scancode, sdl_event.key.repeat);
                broadcast_event(event);
                break;
            }
            case SDL_EventType::SDL_EVENT_KEY_UP:
            {
                KeyReleasedEvent event(sdl_event.key.scancode);
                broadcast_event(event);
                break;
            }
            case SDL_EventType::SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                MouseButtonPressedEvent event(sdl_event.button.button, sdl_event.button.x, sdl_event.button.y);
                broadcast_event(event);
                break;
            }
            case SDL_EventType::SDL_EVENT_MOUSE_BUTTON_UP:
            {
                MouseButtonReleasedEvent event(sdl_event.button.button);
                broadcast_event(event);
                break;
            }
            case SDL_EventType::SDL_EVENT_MOUSE_MOTION:
            {
                MouseMovedEvent event(sdl_event.motion.x, sdl_event.motion.y);
                broadcast_event(event);
                break;
            }
            case SDL_EventType::SDL_EVENT_MOUSE_WHEEL:
            {
                MouseScrolledEvent event(sdl_event.wheel.x, sdl_event.wheel.y);
                broadcast_event(event);
                break;
            }
        }
    }

    return quit_requested;
}

//--------------------------------------------------------------------------------------------------
void Engine::update(float delta_time)
{
    for (auto const& layer : m_layer_stack)
    {
        layer->update_layer(delta_time);
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
        layer->render_layer(renderer);
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

//--------------------------------------------------------------------------------------------------
void Engine::apply_pending_transitions()
{
    for (auto& transition : m_pending_transitions)
    {
        auto anchor = transition.anchor.lock();

        // Find anchor (or use back for pop_layer where anchor is empty)
        auto it = anchor
            ? std::ranges::find(m_layer_stack, anchor)
            : m_layer_stack.end() - 1;

        if (it == m_layer_stack.end()) continue;

        // Cleanup the layer being removed
        (*it)->cleanup_layer();

        if (transition.new_layer)
        {
            // Replace in-place
            *it = std::move(transition.new_layer);
            (*it)->initialize_layer();
        }
        else
        {
            // Pure removal
            m_layer_stack.erase(it);
        }
    }
    m_pending_transitions.clear();
}

//--------------------------------------------------------------------------------------------------
void Engine::fixed_update(float fixed_dt)
{
    for (auto const& layer : m_layer_stack)
    {
        layer->fixed_update_layer(fixed_dt);
    }
}

//--------------------------------------------------------------------------------------------------
void Engine::pop_layer()
{
    assert(!is_rendering());
    if (m_layer_stack.empty()) return;
    m_pending_transitions.push_back({ m_layer_stack.back()->weak_from_this(), nullptr });
}

//--------------------------------------------------------------------------------------------------
void Engine::remove_layer(std::weak_ptr<Layer> layer)
{
    assert(!is_rendering());
    m_pending_transitions.push_back({ std::move(layer), nullptr });
}

} // namespace Core
