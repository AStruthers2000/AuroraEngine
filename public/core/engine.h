////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Main entrypoint for the application.
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CORE_ENGINE_H
#define CORE_ENGINE_H

#include "core/layer.h"

#include <glm/glm.hpp>

#include <cassert>
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

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Main entrypoint for the application. The Engine is the root of the application structure.
///        There should only ever one instance of Engine.
////////////////////////////////////////////////////////////////////////////////////////////////////
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
    /// @brief Pushes a Layer to the Layer stack and returns a weak_ptr to the new Layer.
    ///
    /// @tparam TLayer - The type of Layer to be pushed to the stack.
    ///
    /// @return A std::weak_ptr<TLayer> to the newly created Layer.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template<typename TLayer>
    requires(std::is_base_of_v<Layer, TLayer>)
    std::weak_ptr<TLayer> push_layer()
    {
        assert(!is_cleaning_up());
        assert(!is_rendering());
        auto layer = std::make_shared<TLayer>(get());
        std::weak_ptr<TLayer> result = layer;
        m_pending_layers.push_back(std::move(layer));
        return result;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Removes the top-most Layer from the stack. Deferred until the next frame.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void pop_layer();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Removes a specific Layer from the stack. Deferred until the next frame.
    ///
    /// @param [in] layer - Weak pointer to the Layer to remove.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void remove_layer(std::weak_ptr<Layer> layer);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Replaces the given anchor Layer with a newly constructed Layer of type TLayer.
    ///        The anchor is cleaned up and removed; the new Layer is initialized in its place.
    ///        Deferred until the next frame.
    ///
    /// @tparam TLayer  - The type of Layer to construct as the replacement.
    /// @param  anchor  - The Layer to replace. Must be in the active layer stack.
    ///
    /// @return A std::weak_ptr<TLayer> to the newly created replacement Layer.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template<typename TLayer>
    requires(std::is_base_of_v<Layer, TLayer>)
    std::weak_ptr<TLayer> transition_layer(Layer& anchor)
    {
        assert(!is_cleaning_up());
        assert(!is_rendering());
        auto new_layer = std::make_shared<TLayer>(get());
        std::weak_ptr<TLayer> result = new_layer;
        m_pending_transitions.push_back({ anchor.weak_from_this(), std::move(new_layer) });
        return result;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Sets the fixed timestep used for fixed_update passes.
    ///
    /// @param [in] seconds - Desired fixed timestep in seconds (e.g. 1.f/60.f for 60 Hz).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void set_fixed_timestep(float seconds) { m_fixed_step = seconds; }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return Returns true if the Engine is currently executing a render pass. Adding entities
    ///         or components while this returns true is not allowed.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool is_rendering() const { return m_is_rendering; }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return Returns true if the Engine is currently executing an update pass.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool is_updating() const { return m_is_updating; }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return Returns true if the Engine destructor is running. Treat this as a hard shutdown;
    ///         engine-mutating calls must not be made during cleanup.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool is_cleaning_up() const { return m_is_cleaning_up; }

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
    /// @brief Processes all deferred layer transitions (pop, remove, transition_layer).
    ///        Called once per frame before initialize_pending_layers().
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void apply_pending_transitions();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Runs one fixed-timestep update tick over all layers.
    ///
    /// @param [in] fixed_dt - The fixed timestep interval.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void fixed_update(float fixed_dt);

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
    bool m_is_rendering{ false };
    bool m_is_updating{ false };
    bool m_is_cleaning_up{ false };
    float m_fixed_step{ 1.f / 60.f };
    float m_accumulator{ 0.f };

    std::unique_ptr<Window> m_window;

    struct PendingTransition
    {
        std::weak_ptr<Layer> anchor;   ///< Layer to remove (nullptr anchor = pop_layer).
        std::shared_ptr<Layer> new_layer; ///< Replacement Layer, or nullptr for a pure removal.
    };

    std::vector<PendingTransition>    m_pending_transitions;
    std::vector<std::shared_ptr<Layer>> m_pending_layers;
    std::vector<std::shared_ptr<Layer>> m_layer_stack;
};

} // namespace Core

#endif // CORE_ENGINE_H
