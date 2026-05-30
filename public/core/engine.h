////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Main entrypoint for the application. The Engine is the root of the application structure.
///        There should only ever one instance of Engine.
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ENGINE_ENGINE_H
#define ENGINE_ENGINE_H

#include "core/layer.h"

#include <glm/glm.hpp>

#include <memory>
#include <string>
#include <vector>

namespace Core
{

struct WindowSpecification
{
    /// @brief Title of the window.
    std::string title{ "Aurora Engine" };

    /// @brief "Physical" size of the window on screen. Irrelevant if window is in fullscreen.
    glm::vec2 window_size{ 1920, 1080 };

    /// @brief "Logical" size of the window. Controls the resolution (i.e. size of a pixel).
    glm::vec2 logical_size{ 680, 384 };

    /// @brief Flags passed to SDL_Init() when SDL is initialized. See the documentation of
    ///        SDL_Init() for more information.
    SDL_InitFlags sdl_init_flags{ SDL_INIT_VIDEO };

    /// @brief Flags passed to SDL_CreateWindow() when the window is created. See the documentation
    ///        of SDL_CreateWindow() for more information.
    SDL_WindowFlags sdl_window_flags{ SDL_WINDOW_RESIZABLE };

    /// @brief Flags passed to SDL_SetRenderLogicalPresentation() when the window is created. See
    ///        the documentation of the SDL_RenderLogicalPresentation enum for more information.
    SDL_RendererLogicalPresentation sdl_renderer_mode{ SDL_RendererLogicalPresentation::SDL_LOGICAL_PRESENTATION_LETTERBOX };

    /// @brief Flag passed to SDL_SetRenderVSync() when the window is created. See the documentation
    ///        of SDL_SetRenderVSync() for more information.
    int vsync_flag{ SDL_RENDERER_VSYNC_DISABLED };
};

class Window;

class Engine
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Engine constructor. There should only be one Engine instance. This is asserted in the
    ///        constructor.
    ///
    /// @param [in] window_spec - Window specifications. In the current design, the Engine only
    ///                           creates one Window. The Engine uses these specifications to
    ///                           initialize SDL as well as the window that will be displayed.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    explicit Engine(WindowSpecification const& window_spec);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Engine destructor.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ~Engine();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Main entrypoint into the Engine. Calling this will begin the main game loop. This
    ///        function will only return when the game is closed.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void run();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Stops the Engine from running by breaking out of run().
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void stop();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Pushes a Layer to the Layer stack. The Layer is constructed from the template
    ///        parameter, so callers only need to provide the type of layer in TLayer. This
    ///        structure assumes that there will never be any custom arguments into a Layer's
    ///        constructor.
    ///
    /// @tparam TLayer - The type of Layer to be pushed to the stack.  
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template<typename TLayer>
    requires(std::is_base_of_v<Layer, TLayer>)
    void push_layer()
    {
        m_pending_layers.push_back(std::make_unique<TLayer>(get()));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return Returns the static Engine instance.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static Engine& get();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return Returns a reference to the currently managed window.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    Window& get_window() const
    {
        assert(m_window);
        return *m_window;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return Returns the window size (in pixels) of the managed window. See 
    ///         `SDL_GetWindowSizeInPixels()` for more detail.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    glm::vec2 get_window_size() const;

private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Moves all layers from the pending Layers stack to the active Layers stack. This
    ///        function calls Layer::initialize() for each of the pending Layers. This should be
    ///        called at the beginning of each frame, in case any Layers were added during last
    ///        frame.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void initialize_pending_layers();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Processes all input, draining the SDL event queue. TODO: A lot more is needed here.
    ///
    /// @return Returns true if the Engine should quit (i.e. SDL_EVENT_QUIT was received during this
    ///         frame). Otherwise, false.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool process_input();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Calls Layer::update() for each Layer in the Layer stack.
    ///
    /// @param [in] delta_time - Time since last update.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void update(float delta_time);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Calls Layer::render() for each Layer in the Layer stack. Also handles
    ///        SDL_RenderClear() and SDL_RenderPresent() for swapping between active render buffers.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void render();

    bool m_running = false;
    std::unique_ptr<Window> m_window;

    std::vector<std::unique_ptr<Layer>> m_pending_layers;
    std::vector<std::unique_ptr<Layer>> m_layer_stack;
};

} // namespace Core

#endif // ENGINE_ENGINE_H
