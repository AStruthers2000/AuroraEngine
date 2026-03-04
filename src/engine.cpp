#include "engine/engine.h"

#include "engine/time_keeper.h"
#include "engine/world.h"

namespace AuroraEngine
{

static Engine* s_engine = nullptr;

//--------------------------------------------------------------------------------------------------
Engine::Engine(WindowSpecification const& window_spec)
    : m_window(std::make_unique<Window>(window_spec))
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                 "Error",
                                 "Failed to initialize SDL3!",
                                 nullptr);
    }

    s_engine = this;
}

//--------------------------------------------------------------------------------------------------
Engine::~Engine()
{
    m_managed_world->cleanup();
    m_managed_world.reset();

    m_window->destroy();
    m_window.reset();

    // Quit SDL
    SDL_Quit();
}

//--------------------------------------------------------------------------------------------------
void Engine::initialize(std::unique_ptr<GameWorld> managed_world)
{
    if (managed_world == nullptr)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                 "Error",
                                 "Can't initialize engine with invalid world; please create a valid world instance.",
                                 nullptr);
        return;
    }

    m_window->create();

    // Take ownership of world and initialize it with the SDL state
    m_managed_world = std::move(managed_world);
    m_managed_world->initialize();
    
    m_initialized = true;
}

//--------------------------------------------------------------------------------------------------
void Engine::run()
{
    if (!m_initialized)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                 "Error",
                                 "Can't run Engine; must call initialize() first",
                                 nullptr);
        return;
    }

    // Probably come up with some more official way to track whether the engine is running or not
    bool running = true;

    // Using a variable time step method
//    auto target_frame_time = static_cast<std::uint64_t>((1.f / 144.f) * 1E+9);
    std::uint64_t previous_frame_time = get_current_time();
    while (running)
    {
        std::uint64_t frame_start_time = get_current_time();
        // float delta_time = static_cast<float>(frame_start_time - previous_frame_time) / 1E+09;
        float delta_time = time_delta<TIME_UNITS>(previous_frame_time, frame_start_time);

        running = process_input();
        update(delta_time);
        render();

//        std::uint64_t frame_end_time = get_current_time();
//        // // float frame_time = static_cast<float>(frame_end_time - frame_start_time) / 1'000.f;
//        float frame_time = time_delta<TIME_UNITS>(frame_start_time, frame_end_time);
//        float frame_sleep_time = static_cast<float>(target_frame_time) - frame_time;
//        auto frame_sleep_ns = static_cast<std::int64_t>(frame_sleep_time);
//        if (frame_sleep_ns > 0)
//        {
//            SDL_DelayPrecise(frame_sleep_ns);
//        }
//        else
//        {
//            printf("Frame longer than target\n");
//        }

        previous_frame_time = frame_start_time;
    }
}

Engine &Engine::get()
{
    assert(s_engine);
    return *s_engine;
}

//--------------------------------------------------------------------------------------------------
bool Engine::process_input()
{
    return m_input_subsystem.test();
}

//--------------------------------------------------------------------------------------------------
void Engine::update(float delta_time)
{
    m_managed_world->update(delta_time);
}

//--------------------------------------------------------------------------------------------------
void Engine::render()
{
    // Eventually refactor this to have the window handle rendering entirely
    SDL_Renderer* renderer = m_window->get_sdl_renderer();
    SDL_SetRenderDrawColor(renderer, 20, 10, 30, 255);
    SDL_RenderClear(renderer);

    // Perform all rendering
    m_managed_world->render(renderer);

    // Swap buffers and present
    SDL_RenderPresent(renderer);
}

} // namespace AuroraEngine
