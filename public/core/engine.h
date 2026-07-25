////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Aurora Engine main entry point and application root.
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

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Describes the SDL window to create when constructing the Engine.
///
/// Pass this struct to the Engine constructor to customize the application window. All fields have
/// reasonable defaults so that a minimal Engine can be constructed with a default-initialized
/// WindowSpecification.
////////////////////////////////////////////////////////////////////////////////////////////////////
struct WindowSpecification
{
    /// @brief Title bar text of the application window.
    std::string title{ "Aurora Engine" };

    /// @brief Physical size of the window on the monitor in screen coordinates. Has no effect if
    ///        the window is fullscreen.
    glm::vec2 window_size{ 1920, 1080 };

    /// @brief Logical (virtual) resolution of the renderer. A logical pixel maps to however many
    ///        physical pixels are needed to fill the window at the current physical size.
    ///        Controls the effective game resolution independently of the window size.
    glm::vec2 logical_size{ 680, 384 };

    /// @brief SDL subsystem flags passed to @c SDL_Init(). See the SDL documentation for
    ///        @c SDL_InitFlags.
    SDL_InitFlags sdl_init_flags{ SDL_INIT_VIDEO };

    /// @brief Window creation flags passed to @c SDL_CreateWindow(). See the SDL documentation
    ///        for @c SDL_WindowFlags.
    SDL_WindowFlags sdl_window_flags{ SDL_WINDOW_RESIZABLE };

    /// @brief Logical-presentation mode for the SDL renderer. Controls how the logical resolution
    ///        is letterboxed or stretched to fill the physical window. See the SDL documentation
    ///        for @c SDL_RendererLogicalPresentation.
    SDL_RendererLogicalPresentation sdl_renderer_mode{
        SDL_RendererLogicalPresentation::SDL_LOGICAL_PRESENTATION_LETTERBOX };

    /// @brief VSync mode flag passed to @c SDL_SetRenderVSync(). Use @c SDL_RENDERER_VSYNC_DISABLED
    ///        to disable VSync, or @c 1 to enable. See the SDL documentation for valid values.
    int vsync_flag{ SDL_RENDERER_VSYNC_DISABLED };
};

class Event;
class Window;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Application root. The Engine owns the Layer stack and drives the main game loop.
///
/// Engine is the single point of entry for an Aurora Engine application. There must be exactly one
/// Engine instance per process. Construct it in @c main(), push at least one Layer via
/// push_layer<T>(), then call run() to start the main loop. Call stop() (e.g. from a key event)
/// to exit gracefully.
///
/// @code
/// int main()
/// {
///     Core::Engine engine{ Core::WindowSpecification{} };
///     engine.push_layer<MyGameLayer>();
///     engine.run();
/// }
/// @endcode
////////////////////////////////////////////////////////////////////////////////////////////////////
class Engine
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Construction & Destruction
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Engine constructor.
    ///
    /// @note  Exactly one Engine instance must exist for the lifetime of the application. The
    ///        constructor asserts that no other Engine instance is live.
    ///
    /// @param [in] window_spec - Window and SDL initialization parameters.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    explicit Engine(WindowSpecification const& window_spec);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Engine destructor. Cleans up all active Layers in reverse stack order, then
    ///        shuts down SDL.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ~Engine();


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Core Control
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Starts the main game loop. Blocks until stop() is called or the application window
    ///        is closed. Layers pushed before this call are initialized on the first frame.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void run();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Requests the Engine to exit. The current frame completes, then run() returns.
    ///
    /// @note  Safe to call from any Layer or Entity on_update() / on_event() override.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void stop();


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Layer Management
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Constructs and pushes a new Layer of type TLayer onto the top of the Layer stack.
    ///        The Layer is initialized at the start of the next frame.
    ///
    /// @note  Must not be called during a render or cleanup pass.
    ///
    /// @tparam TLayer The Layer type to construct. Must be derived from Layer.
    ///
    /// @return A @c std::weak_ptr<TLayer> to the newly created Layer.
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
    /// @brief Removes the top-most Layer from the stack. Cleanup and removal are deferred until
    ///        the next frame.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void pop_layer();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Removes a specific Layer from the stack. Cleanup and removal are deferred until
    ///        the next frame.
    ///
    /// @param [in] layer - Weak pointer to the Layer to remove.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void remove_layer(std::weak_ptr<Layer> layer);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Replaces an existing Layer with a new Layer of type TLayer. The anchor Layer is
    ///        cleaned up and removed; the replacement is initialized in its place. Deferred until
    ///        the next frame.
    ///
    /// @tparam TLayer  The Layer type to construct as the replacement. Must be derived from Layer.
    ///
    /// @param [in] anchor - The active Layer to replace.
    ///
    /// @return A @c std::weak_ptr<TLayer> to the newly created replacement Layer.
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
    // State & Queries
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Sets the fixed timestep used for fixed-update passes.
    ///
    /// @param [in] seconds - Desired fixed timestep in seconds (e.g. @c 1.f/60.f for 60 Hz).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void set_fixed_timestep(float seconds) { m_fixed_step = seconds; }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return The static singleton Engine instance. Asserts that one exists.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static Engine& get();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return A reference to the managed application window.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    Window& get_window() const
    {
        assert(m_window);
        return *m_window;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return The window size in pixels. See @c SDL_GetWindowSizeInPixels() for details.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    glm::vec2 get_window_size() const;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Broadcasts an event to all Layers in reverse stack order (top Layer receives it
    ///        first). Each Layer's propagate_event_down() is called, distributing the event to
    ///        all Entities on that Layer before calling on_event() on the Layer itself.
    ///
    /// @param [in] event - The event to broadcast.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void broadcast_event(Event& event);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return @c true if the Engine is currently executing a render pass. Adding Entities or
    ///         Components while this returns @c true is not allowed.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool is_rendering()    const { return m_is_rendering;    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return @c true if the Engine is currently executing an update pass.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool is_updating()     const { return m_is_updating;     }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return @c true if the Engine destructor is running. Engine-mutating calls must not be
    ///         made once this returns @c true.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool is_cleaning_up()  const { return m_is_cleaning_up;  }

private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Engine Internals
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Processes all deferred layer operations (pop_layer, remove_layer, transition_layer)
    ///        accumulated during the previous frame. Calls cleanup_layer() on any Layers being
    ///        removed, then inserts replacement Layers into the pending queue.
    ///
    /// @note  Called once per frame at the top of run(), before initialize_pending_layers().
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void apply_pending_transitions();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Moves all Layers from the pending queue into the active Layer stack and calls
    ///        initialize_layer() on each. Called once per frame to ensure Layers pushed during the
    ///        previous frame are ready before the next update.
    ///
    /// @note  Called once per frame, after apply_pending_transitions().
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void initialize_pending_layers();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Drains the SDL event queue and broadcasts each event to the Layer stack via
    ///        broadcast_event().
    ///
    /// @note  SDL_EVENT_QUIT is translated into a call to stop().
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void process_input();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Executes one fixed-timestep update tick over all Layers. Calls fixed_update_layer()
    ///        on each active Layer in stack order.
    ///
    /// @param [in] fixed_dt - The fixed timestep in seconds.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void fixed_update(float fixed_dt);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Executes the per-frame update. Calls update_layer() on each active Layer in stack
    ///        order.
    ///
    /// @param [in] delta_time - Seconds elapsed since the last frame.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void update(float delta_time);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Executes the per-frame render pass. Clears the SDL renderer, calls render_layer()
    ///        on each active Layer in stack order, then presents the rendered frame.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void render();

    /// @brief Set false by stop() to exit run().
    bool m_running{ false };

    /// @brief True during the render pass.
    bool m_is_rendering{ false };

    /// @brief True during the update pass.
    bool m_is_updating{ false };

    /// @brief True inside the Engine destructor.
    bool m_is_cleaning_up{ false };

    /// @brief Fixed timestep interval in seconds.
    float m_fixed_step{ 1.f / 60.f };

    /// @brief Accumulated unstepped time for fixed updates.
    float m_accumulator{ 0.f };

    /// @brief Reentrancy guard for broadcast_event().
    std::uint32_t m_broadcast_depth{ 0 };

    /// @brief The managed SDL window.
    std::unique_ptr<Window> m_window;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Represents a deferred layer operation (pop, remove, or replace).
    ///
    /// When @c anchor is non-null, it identifies the Layer to remove. When @c new_layer is
    /// non-null, it is inserted after @c anchor is removed (transition). A null @c new_layer with
    /// a null @c anchor means pop the top-most Layer.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    struct PendingTransition
    {
        /// @brief Layer to remove (empty = pop top).
        std::weak_ptr<Layer>   anchor;
        /// @brief Replacement Layer, or nullptr for pure removal.
        std::shared_ptr<Layer> new_layer;
    };

    /// @brief Deferred layer operations queue.
    std::vector<PendingTransition>      m_pending_transitions;

    /// @brief Layers awaiting initialization.
    std::vector<std::shared_ptr<Layer>> m_pending_layers;

    /// @brief Active Layer stack (bottom to top).
    std::vector<std::shared_ptr<Layer>> m_layer_stack;
};

} // namespace Core

#endif // CORE_ENGINE_H
