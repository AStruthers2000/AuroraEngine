////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief A self-contained Entity that clips its children to a fixed-size viewport and exposes
///        vertical scrolling via mouse wheel and a draggable scrollbar.
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CORE_ENTITIES_UI_SCROLLABLE_VIEWPORT_H
#define CORE_ENTITIES_UI_SCROLLABLE_VIEWPORT_H

#include "core/entities/ui/ui_element.h"
#include "core/events/mouse_input_events.h"

#include <glm/glm.hpp>

namespace Core
{
class TransformComponent;
}

namespace Core::UI
{
class Panel;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief A self-contained Entity that scissor-clips its children to a fixed-size viewport and
///        exposes vertical scrolling via mouse wheel and a draggable scrollbar.
///
/// ScrollableViewport is content-agnostic: it knows nothing about what kind of children it hosts
/// (Labels, Buttons, Panels, etc.) and tracks only a single scalar "content height" supplied by
/// the owner. Children are added directly via Entity::add_child_entity<T>() like any other
/// Entity. This is intended to be composed by higher-level widgets (ScrollableLog, and eventually
/// things like dropdowns or settings navigation).
///
/// @note  Vertical scrolling only. The widget's own TransformComponent size, @c viewport_size
///        covers the full control, including the scrollbar strip on the right edge. Content
///        children should stay within [0, get_content_width()) horizontally to avoid rendering
///        under the scrollbar.
////////////////////////////////////////////////////////////////////////////////////////////////////
class ScrollableViewport : public UIElement
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Types
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Construction parameters for ScrollableViewport.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    struct Configuration : UIElement::Configuration
    {
        /// @brief Full size of the control, including the scrollbar strip on the right edge.
        glm::vec2 viewport_size{ 200.f, 200.f };
        /// @brief Width of the scrollbar track/handle strip, in pixels.
        float scrollbar_width{ 12.f };
        /// @brief Minimum handle height, in pixels, regardless of content-to-viewport ratio.
        float min_handle_size{ 20.f };
        /// @brief Content pixels scrolled per unit of mouse wheel movement.
        float wheel_scroll_step{ 40.f };
        /// @brief Scrollbar track fill color.
        SDL_Color track_color{ 229, 228, 226, 255 };
        /// @brief Scrollbar handle fill color at rest.
        SDL_Color handle_color{ 150, 150, 150, 255 };
        /// @brief Scrollbar handle fill color while hovered or dragged.
        SDL_Color handle_hover_color{ 100, 100, 100, 255 };
    };


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Construction & Destruction
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Constructs a ScrollableViewport.
    ///
    /// @param [in] owner  - The Layer or parent Entity that will own this ScrollableViewport.
    /// @param [in] config - Optional construction parameters.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ScrollableViewport(Entity::Owner owner, Configuration const& config = {});


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Virtual Lifecycle Hook Overrides
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Caches the scrollbar handle's TransformComponent (valid now that child Panels have
    ///        completed their own awake pass) and applies the initial scrollbar layout.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void on_start() override;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Pushes a scissor clip rect covering this viewport's world bounds, intersected with
    ///        any clip rect already active (nested-viewport safe), before children render.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void on_render(SDL_Renderer* renderer) override;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Restores whatever clip rect was active before on_render(), now that this viewport's
    ///        children have finished rendering.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void on_post_render(SDL_Renderer* renderer) override;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Routes mouse wheel/move/press/release events to scroll and scrollbar-drag handling.
    ///
    /// @param [in] event - The event being propagated.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void on_event(Event& event) override;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Runtime API
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Sets the total scrollable content height. Clamps the current scroll offset and
    ///        recomputes the scrollbar handle's size/position, hiding the scrollbar entirely if
    ///        the content now fits within the viewport.
    ///
    /// @param [in] content_height - New total content height, in pixels.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void set_content_height(float content_height);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return The current total scrollable content height, in pixels.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    float get_content_height() const { return m_content_height; }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Sets the current scroll offset, clamped to [0, get_max_scroll_offset()].
    ///
    /// @param [in] offset - Desired scroll offset, in content-space pixels.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void set_scroll_offset(float offset);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return The current scroll offset, in content-space pixels.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    float get_scroll_offset() const { return m_scroll_offset; }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return The maximum valid scroll offset, i.e. @c content_height - viewport height, clamped
    ///         to zero when content fits within the viewport.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    float get_max_scroll_offset() const;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return The full control size, including the scrollbar strip.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    glm::vec2 get_viewport_size() const { return m_viewport_size; }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @return Usable width for content children, i.e. @c viewport_size.x minus the scrollbar
    ///         strip width.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    float get_content_width() const { return m_viewport_size.x - m_scrollbar_width; }

private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Event Handlers
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Updates the cached last-known mouse position, drives an in-progress handle drag
    ///        (if any), and tracks hover state over the handle for resolve_handle_color().
    ///
    /// @param [in] event - The mouse-moved event.
    ///
    /// @return @c true if a handle drag is in progress (event consumed).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool on_mouse_moved(MouseMovedEvent& event);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Begins a scrollbar handle drag if the left mouse button was pressed on the handle.
    ///
    /// @param [in] event - The mouse-button-pressed event.
    ///
    /// @return @c true if a drag was started (event consumed).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool on_mouse_pressed(MouseButtonPressedEvent& event);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Ends an in-progress scrollbar handle drag on left mouse button release.
    ///
    /// @param [in] event - The mouse-button-released event.
    ///
    /// @return @c true if a drag was ended (event consumed).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool on_mouse_released(MouseButtonReleasedEvent& event);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Applies wheel-scroll input to the scroll offset, gated on the mouse being within
    ///        this viewport's bounds (see @c m_last_mouse_position).
    ///
    /// @param [in] event - The mouse-scrolled event.
    ///
    /// @return @c true if the scroll offset changed (event consumed).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool on_mouse_scrolled(MouseScrolledEvent& event);


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Queries
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @param [in] point - Point to test, in world space.
    ///
    /// @return @c true if @p point falls within this viewport's own world bounds.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool is_point_within_viewport(glm::vec2 const& point) const;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @param [in] point - Point to test, in world space.
    ///
    /// @return @c true if @p point falls within the scrollbar handle's world bounds.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool is_point_within_handle(glm::vec2 const& point) const;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Scrollbar Layout
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Recomputes handle size/position from the current content height, viewport size, and
    ///        scroll offset. Hides the track/handle entirely when content fits within the
    ///        viewport. Called whenever content height or scroll offset changes - never polled.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void recompute_scrollbar_layout();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Applies @c handle_hover_color while hovered or dragged, @c handle_color otherwise.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void resolve_handle_color();

    /// @brief Full control size, including the scrollbar strip. See Configuration::viewport_size.
    glm::vec2 m_viewport_size{};

    /// @brief Width of the scrollbar track/handle strip, in pixels.
    float m_scrollbar_width{};

    /// @brief Minimum handle height, in pixels, regardless of content-to-viewport ratio.
    float m_min_handle_size{};

    /// @brief Content pixels scrolled per unit of mouse wheel movement.
    float m_wheel_scroll_step{};

    /// @brief Scrollbar handle fill color at rest.
    SDL_Color m_handle_color{};

    /// @brief Scrollbar handle fill color while hovered or dragged.
    SDL_Color m_handle_hover_color{};

    /// @brief Total scrollable content height, in pixels. See set_content_height().
    float m_content_height{ 0.f };

    /// @brief Current scroll offset, in content-space pixels. See set_scroll_offset().
    float m_scroll_offset{ 0.f };

    /// @brief Last mouse position seen via MouseMovedEvent - MouseScrolledEvent carries no
    ///        position of its own, so wheel-scroll hover gating relies on this cached value.
    glm::vec2 m_last_mouse_position{};

    /// @brief Whether the mouse is currently hovering the scrollbar handle.
    bool m_handle_hovered{ false };

    /// @brief Whether the scrollbar handle is currently being dragged.
    bool m_dragging_handle{ false };

    /// @brief Mouse y position, in world space, at the start of the current handle drag.
    float m_drag_start_mouse_y{};

    /// @brief Scroll offset at the start of the current handle drag.
    float m_drag_start_scroll_offset{};

    /// @brief Cached TransformComponent on this Entity, used for world-space bounds checks.
    std::weak_ptr<TransformComponent> m_transform{};

    /// @brief The scrollbar track Panel, child of this Entity.
    std::weak_ptr<Panel> m_track{};

    /// @brief The scrollbar handle Panel, child of this Entity.
    std::weak_ptr<Panel> m_handle{};

    /// @brief Cached TransformComponent on @c m_handle, used to resize/reposition the handle.
    std::weak_ptr<TransformComponent> m_handle_transform{};

    /// @brief Clip-rect state saved across on_render()/on_post_render() so a prior (outer, nested)
    ///        clip rect can be restored rather than clobbered.
    bool m_had_previous_clip{ false };

    /// @brief The clip rect that was active before on_render() pushed this viewport's own clip,
    ///        valid only when @c m_had_previous_clip is @c true.
    SDL_Rect m_previous_clip_rect{};
};

} // namespace Core::UI

#endif // CORE_ENTITIES_UI_SCROLLABLE_VIEWPORT_H
