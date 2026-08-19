////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "core/entities/ui/button.h"

#include "core/components/transform_component.h"
#include "core/entities/ui/panel.h"
#include "core/events/event_dispatcher.h"

namespace Core::UI
{

//--------------------------------------------------------------------------------------------------
Button::Button(Entity::Owner owner, Configuration const& config)
    : UIElement(owner, config)
    , m_on_button_pressed_callback(config.on_button_pressed)
    , m_on_button_released_callback(config.on_button_released)
    , m_on_button_hovered_callback(config.on_button_hovered)
    , m_on_button_unhovered_callback(config.on_button_unhovered)
    , m_clicked_decoration_type(config.clicked_decoration)
    , m_hovered_decoration_type(config.hovered_decoration)
    , m_clicked_color(config.clicked_color)
    , m_hovered_color(config.hovered_color)
    , m_original_panel_color(config.button_color)
    , m_original_border_color(config.border_color)
{
    TransformComponent::Configuration transform_cfg;
    transform_cfg.position = config.offset;
    transform_cfg.size = config.button_size;
    m_transform = add_component<TransformComponent>("", transform_cfg);

    Panel::Configuration panel_cfg;
    panel_cfg.update_order  = config.update_order;
    panel_cfg.self_anchor   = config.self_anchor;
    panel_cfg.parent_anchor = config.parent_anchor;
    panel_cfg.offset        = { 0, 0 };
    panel_cfg.panel_size    = config.button_size;
    panel_cfg.panel_color   = config.button_color;
    panel_cfg.has_border    = true;
    panel_cfg.border_size   = config.border_size;
    panel_cfg.border_color  = config.border_color;
    m_panel                 = add_child_entity<Panel>(panel_cfg, "main-body");

    Panel::Configuration highlighted_effect_cfg{ panel_cfg };
    highlighted_effect_cfg.panel_color  = config.hovered_color;
    highlighted_effect_cfg.has_border   = false;
    highlighted_effect_cfg.border_size  = { 0, 0 };
    highlighted_effect_cfg.border_color = { 0, 0, 0, 0 };
    m_highlighted_effect                = add_child_entity<Panel>(highlighted_effect_cfg, "highlight-effect");

    Panel::Configuration clicked_effect_cfg{ panel_cfg };
    clicked_effect_cfg.panel_color  = config.clicked_color;
    clicked_effect_cfg.has_border   = false;
    clicked_effect_cfg.border_size  = { 0, 0 };
    clicked_effect_cfg.border_color = { 0, 0, 0, 0 };
    m_clicked_effect                = add_child_entity<Panel>(clicked_effect_cfg, "click-effect");
}

//--------------------------------------------------------------------------------------------------
void Button::on_start()
{
    if (auto clicked_effect = m_clicked_effect.lock())
    {
        clicked_effect->set_disabled();
    }

    if (auto highlighted_effect = m_highlighted_effect.lock())
    {
        highlighted_effect->set_disabled();
    }

    resolve_decoration_state();
}

//--------------------------------------------------------------------------------------------------
void Button::on_event(Event& event)
{
    EventDispatcher dispatcher(event);
    dispatcher.dispatch<MouseMovedEvent>([this](MouseMovedEvent& event){ return this->on_mouse_moved(event); });
    dispatcher.dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent& event){ return this->on_mouse_pressed(event); });
    dispatcher.dispatch<MouseButtonReleasedEvent>([this](MouseButtonReleasedEvent& event){ return this->on_mouse_released(event); });
}

//--------------------------------------------------------------------------------------------------
bool Button::on_mouse_moved(MouseMovedEvent& event)
{
    bool const prev_hovered{ m_is_hovered };
    bool const hovered = is_mouse_within_extents(event.get_mouse_position());

    // Rising edge of hovered
    if (!prev_hovered && hovered)
    {
        if (m_on_button_hovered_callback)
        {
            m_on_button_hovered_callback(event);
        }
    }
    // Falling edge of hovered
    else if (prev_hovered && !hovered)
    {
        if (m_on_button_unhovered_callback)
        {
            m_on_button_unhovered_callback(event);
        }
    }

    m_is_hovered = hovered;
    resolve_decoration_state();

    return m_is_hovered;
}

//--------------------------------------------------------------------------------------------------
bool Button::on_mouse_pressed(MouseButtonPressedEvent& event)
{
    bool left_mouse_button = event.get_mouse_button() == 1;
    if (left_mouse_button && m_is_hovered)
    {
        m_is_clicked = true;
        resolve_decoration_state();

        if (m_on_button_pressed_callback)
        {
            m_on_button_pressed_callback(event);
        }
        
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
bool Button::on_mouse_released(MouseButtonReleasedEvent& event)
{
    bool left_mouse_button = event.get_mouse_button() == 1;
    if (left_mouse_button && m_is_clicked)
    {
        m_is_clicked = false;
        resolve_decoration_state();

        if (m_on_button_released_callback)
        {
            m_on_button_released_callback(event);
        }

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
bool Button::is_mouse_within_extents(glm::vec2 const& mouse_position)
{
    bool mouse_in_extents{ false };
    if (auto transform = m_transform.lock())
    {
        glm::vec2 top_left = transform->get_world_position();
        glm::vec2 bottom_right = top_left + (transform->get_size() * transform->get_world_scale());

        bool in_horizontal = mouse_position.x >= top_left.x && mouse_position.x <= bottom_right.x;
        bool in_vertical = mouse_position.y >= top_left.y && mouse_position.y <= bottom_right.y;

        mouse_in_extents = in_horizontal && in_vertical;
    }

    return mouse_in_extents;
}

//--------------------------------------------------------------------------------------------------
void Button::resolve_decoration_state()
{
    if (m_is_hovered && m_is_clicked)
    {
        deactivate_hovered_decoration();
        activate_clicked_decoration();
    }
    else if (m_is_hovered && !m_is_clicked)
    {
        activate_hovered_decoration();
        deactivate_clicked_decoration();
    }
    else if (!m_is_hovered && m_is_clicked)
    {
        deactivate_hovered_decoration();
        activate_clicked_decoration();
    }
    else
    {
        deactivate_hovered_decoration();
        deactivate_clicked_decoration();
    }
}

//--------------------------------------------------------------------------------------------------
void Button::activate_clicked_decoration()
{
    switch (m_clicked_decoration_type)
    {
        case EDecorationType::HIGHLIGHTED:
        {
            if (auto clicked_effect = m_clicked_effect.lock())
            {
                clicked_effect->set_active();
            }
            break;
        }
        case EDecorationType::COLOR_CHANGE_BORDER:
        {
            if (auto panel = m_panel.lock())
            {
                panel->set_border_color(m_clicked_color);
            }
            break;
        }
        case EDecorationType::COLOR_CHANGE_PANEL:
        {
            if (auto panel = m_panel.lock())
            {
                panel->set_panel_color(m_clicked_color);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
void Button::deactivate_clicked_decoration()
{
    switch (m_clicked_decoration_type)
    {
        case EDecorationType::HIGHLIGHTED:
        {
            if (auto clicked_effect = m_clicked_effect.lock())
            {
                clicked_effect->set_disabled();
            }
            break;
        }
        case EDecorationType::COLOR_CHANGE_BORDER:
        {
            if (auto panel = m_panel.lock())
            {
                panel->set_border_color(m_original_border_color);
            }
            break;
        }
        case EDecorationType::COLOR_CHANGE_PANEL:
        {
            if (auto panel = m_panel.lock())
            {
                panel->set_panel_color(m_original_panel_color);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
void Button::activate_hovered_decoration()
{
    switch (m_hovered_decoration_type)
    {
        case EDecorationType::HIGHLIGHTED:
        {
            if (auto highlighted_effect = m_highlighted_effect.lock())
            {
                highlighted_effect->set_active();
            }
            break;
        }
        case EDecorationType::COLOR_CHANGE_BORDER:
        {
            if (auto panel = m_panel.lock())
            {
                panel->set_border_color(m_hovered_color);
            }
            break;
        }
        case EDecorationType::COLOR_CHANGE_PANEL:
        {
            if (auto panel = m_panel.lock())
            {
                panel->set_panel_color(m_hovered_color);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
void Button::deactivate_hovered_decoration()
{
    switch (m_hovered_decoration_type)
    {
        case EDecorationType::HIGHLIGHTED:
        {
            if (auto highlighted_effect = m_highlighted_effect.lock())
            {
                highlighted_effect->set_disabled();
            }
            break;
        }
        case EDecorationType::COLOR_CHANGE_BORDER:
        {
            if (auto panel = m_panel.lock())
            {
                panel->set_border_color(m_original_border_color);
            }
            break;
        }
        case EDecorationType::COLOR_CHANGE_PANEL:
        {
            if (auto panel = m_panel.lock())
            {
                panel->set_panel_color(m_original_panel_color);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

} // namespace Core::UI
