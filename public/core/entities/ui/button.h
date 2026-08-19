////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief A self-contained Entity that acts as a button, exposing callbacks for hovered and clicked
///        events.
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CORE_ENTITIES_UI_BUTTON_H
#define CORE_ENTITIES_UI_BUTTON_H

#include "core/entities/ui/ui_element.h"
#include "core/events/mouse_input_events.h"

#include <glm/glm.hpp>

#include <functional>

namespace Core
{
class TransformComponent;
} // namespace Core

namespace Core::UI
{
class Panel;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief A self-contained Entity that acts as a clickable button, composing a background Panel
///        with optional hover/click decoration effects and exposing state-change callbacks.
///
/// Button composes three child Panels: the main body (@c button_color / @c border_color), an
/// optional hover-highlight overlay, and an optional click overlay. Each decoration type (@c
/// hovered_decoration / @c clicked_decoration) selects whether its overlay Panel is shown, or
/// whether the main body's fill/border color changes instead. Construct it via
/// Layer::add_entity<Button>() or add_child_entity<Button>() and pass a Configuration to
/// customize size, colors, decoration behavior, and callbacks.
///
/// @note  Callbacks (@c on_button_pressed, @c on_button_released, @c on_button_hovered, @c
///        on_button_unhovered) are optional. Assign one to react to state changes without
///        deriving a new Entity subclass.
////////////////////////////////////////////////////////////////////////////////////////////////////
class Button : public UIElement
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Types
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Selects how a Button visually reacts to a hover or click state. Set independently
    ///        for @c hovered_decoration and @c clicked_decoration in Configuration.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    enum class EDecorationType : std::uint8_t
    {
        NONE,                 ///< No visual change for this state.
        HIGHLIGHTED,          ///< Shows a dedicated overlay Panel on top of the button body.
        COLOR_CHANGE_BORDER,  ///< Recolors the button body's border for the state's duration.
        COLOR_CHANGE_PANEL,   ///< Recolors the button body's fill for the state's duration.
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Construction parameters for Button.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    struct Configuration : UIElement::Configuration
    {
        /// @brief Size of the button's clickable body, in pixels.
        glm::vec2 button_size{ 100, 100 };
        /// @brief Fill color of the button body in its resting state.
        SDL_Color button_color{ 255, 255, 255, 255 };
        /// @brief Border thickness of the button body, in pixels.
        glm::vec2 border_size{ 5, 5 };
        /// @brief Border color of the button body in its resting state.
        SDL_Color border_color{ 54, 69, 79, 255 };
        /// @brief Color used for the clicked decoration (overlay or recolor target, depending
        ///        on @c clicked_decoration).
        SDL_Color clicked_color{ 54, 69, 79, 128 };
        /// @brief Color used for the hovered decoration (overlay or recolor target, depending
        ///        on @c hovered_decoration).
        SDL_Color hovered_color{ 255, 255, 255, 128 };

        /// @brief How the button reacts visually while clicked. See EDecorationType.
        EDecorationType clicked_decoration{ EDecorationType::HIGHLIGHTED };
        /// @brief How the button reacts visually while hovered. See EDecorationType.
        EDecorationType hovered_decoration{ EDecorationType::HIGHLIGHTED };

        /// @brief Optional callback invoked when the left mouse button is pressed while
        ///        hovering this button.
        std::function<void(MouseButtonPressedEvent const&)> on_button_pressed{ nullptr };
        /// @brief Optional callback invoked when the left mouse button is released after this
        ///        button was clicked.
        std::function<void(MouseButtonReleasedEvent const&)> on_button_released{ nullptr };
        /// @brief Optional callback invoked when the mouse cursor enters this button's bounds.
        std::function<void(MouseMovedEvent const&)> on_button_hovered{ nullptr };
        /// @brief Optional callback invoked when the mouse cursor leaves this button's bounds.
        std::function<void(MouseMovedEvent const&)> on_button_unhovered{ nullptr };
    };


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Construction & Destruction
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Constructs a Button.
    ///
    /// @param [in] owner  - The Layer or parent Entity that will own this Button.
    /// @param [in] config - Optional construction parameters.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    Button(Entity::Owner owner, Configuration const& config = {});


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Virtual Lifecycle Hook Overrides
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Caches the main body Panel's Transform and disables the click/hover overlay Panels
    ///        until a decoration needs them. Called once after all child Panels have awoken.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void on_start() override;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Routes mouse move/press/release events to the button's hover and click handling.
    ///
    /// @param [in] event - The event being propagated.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void on_event(Event& event) override;

private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Event Handlers
    //
    // Bound to the mouse events dispatched from on_event() via EventDispatcher.
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Updates hovered state from the button's on-screen extents and fires the
    ///        hover/unhover callbacks on the rising/falling edge.
    ///
    /// @note  Called by: on_event() via EventDispatcher.
    ///
    /// @param [in] event - The mouse-moved event being dispatched.
    ///
    /// @return @c true if the mouse is within the button's extents, marking the event handled.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool on_mouse_moved(MouseMovedEvent& event);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Marks the button clicked and fires the pressed callback, if the left mouse button
    ///        was pressed while hovered.
    ///
    /// @note  Called by: on_event() via EventDispatcher.
    ///
    /// @param [in] event - The mouse-button-pressed event being dispatched.
    ///
    /// @return @c true if the press landed on this button, marking the event handled.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool on_mouse_pressed(MouseButtonPressedEvent& event);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Clears the clicked state and fires the released callback, if this button was
    ///        previously clicked.
    ///
    /// @note  Called by: on_event() via EventDispatcher.
    ///
    /// @param [in] event - The mouse-button-released event being dispatched.
    ///
    /// @return @c true if this button was clicked, marking the event handled.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool on_mouse_released(MouseButtonReleasedEvent& event);


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Queries
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Tests whether a world-space point falls within the main body Panel's bounds.
    ///
    /// @param [in] mouse_position - World-space mouse position to test.
    ///
    /// @return @c true if the position is within the button's extents.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool is_mouse_within_extents(glm::vec2 const& mouse_position);


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Decoration State Management
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Applies the correct combination of hover/click decoration based on the current
    ///        @c m_is_hovered and @c m_is_clicked flags. Clicked decoration takes priority when
    ///        both are true.
    ///
    /// @note  Called by: on_mouse_moved(), on_mouse_pressed(), on_mouse_released(), on_start().
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void resolve_decoration_state();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Activates the clicked decoration per @c m_clicked_decoration_type: shows the click
    ///        overlay Panel, or recolors the main body, depending on the type.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void activate_clicked_decoration();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Reverses activate_clicked_decoration(): hides the click overlay Panel, or restores
    ///        the main body's original color, depending on @c m_clicked_decoration_type.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void deactivate_clicked_decoration();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Activates the hovered decoration per @c m_hovered_decoration_type: shows the hover
    ///        overlay Panel, or recolors the main body, depending on the type.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void activate_hovered_decoration();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Reverses activate_hovered_decoration(): hides the hover overlay Panel, or restores
    ///        the main body's original color, depending on @c m_hovered_decoration_type.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void deactivate_hovered_decoration();


    /// @brief The main body Panel: the button's background and border.
    std::weak_ptr<Panel> m_panel{};

    /// @brief Overlay Panel shown for the clicked decoration when @c m_clicked_decoration_type
    ///        is HIGHLIGHTED.
    std::weak_ptr<Panel> m_clicked_effect{};

    /// @brief Overlay Panel shown for the hovered decoration when @c m_hovered_decoration_type
    ///        is HIGHLIGHTED.
    std::weak_ptr<Panel> m_highlighted_effect{};

    /// @brief Cached Transform.
    std::weak_ptr<TransformComponent> m_transform{};

    /// @brief Optional callback invoked when the button is pressed. Copied from Configuration.
    std::function<void(MouseButtonPressedEvent const&)> m_on_button_pressed_callback{ nullptr };

    /// @brief Optional callback invoked when the button is released. Copied from Configuration.
    std::function<void(MouseButtonReleasedEvent const&)> m_on_button_released_callback{ nullptr };

    /// @brief Optional callback invoked when the button becomes hovered. Copied from
    ///        Configuration.
    std::function<void(MouseMovedEvent const&)> m_on_button_hovered_callback{ nullptr };

    /// @brief Optional callback invoked when the button stops being hovered. Copied from
    ///        Configuration.
    std::function<void(MouseMovedEvent const&)> m_on_button_unhovered_callback{ nullptr };

    /// @brief Whether the mouse cursor is currently within the button's extents.
    bool m_is_hovered{ false };

    /// @brief Whether the button is currently held down (pressed but not yet released).
    bool m_is_clicked{ false };

    /// @brief Decoration behavior applied while clicked. Copied from Configuration.
    EDecorationType const m_clicked_decoration_type{ EDecorationType::NONE };

    /// @brief Decoration behavior applied while hovered. Copied from Configuration.
    EDecorationType const m_hovered_decoration_type{ EDecorationType::NONE };

    /// @brief Color applied for the clicked decoration. Copied from Configuration.
    SDL_Color const m_clicked_color{};

    /// @brief Color applied for the hovered decoration. Copied from Configuration.
    SDL_Color const m_hovered_color{};

    /// @brief The main body's resting fill color, restored when a color-change decoration
    ///        deactivates.
    SDL_Color const m_original_panel_color{};

    /// @brief The main body's resting border color, restored when a color-change decoration
    ///        deactivates.
    SDL_Color const m_original_border_color{};
};

} // namespace Core::UI

#endif // CORE_ENTITIES_UI_BUTTON_H
