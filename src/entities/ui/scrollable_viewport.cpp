////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "core/entities/ui/scrollable_viewport.h"

#include "core/components/transform_component.h"
#include "core/entities/ui/panel.h"
#include "core/events/event_dispatcher.h"

#include <algorithm>

namespace Core::UI
{

//--------------------------------------------------------------------------------------------------
ScrollableViewport::ScrollableViewport(Entity::Owner owner, Configuration const& config)
    : UIElement(owner, config)
    , m_viewport_size(config.viewport_size)
    , m_scrollbar_width(config.scrollbar_width)
    , m_min_handle_size(config.min_handle_size)
    , m_wheel_scroll_step(config.wheel_scroll_step)
    , m_handle_color(config.handle_color)
    , m_handle_hover_color(config.handle_hover_color)
{
    TransformComponent::Configuration transform_cfg;
    transform_cfg.position = config.offset;
    transform_cfg.size = config.viewport_size;
    m_transform = add_component<TransformComponent>("", transform_cfg);

    float const track_x = config.viewport_size.x - config.scrollbar_width;

    Panel::Configuration track_cfg;
    track_cfg.self_anchor   = Anchor::TopLeft;
    track_cfg.parent_anchor = Anchor::TopLeft;
    track_cfg.offset        = { track_x, 0.f };
    track_cfg.panel_size    = { config.scrollbar_width, config.viewport_size.y };
    track_cfg.panel_color   = config.track_color;
    m_track = add_child_entity<Panel>(track_cfg, "scrollbar-track");

    Panel::Configuration handle_cfg;
    handle_cfg.self_anchor   = Anchor::TopLeft;
    handle_cfg.parent_anchor = Anchor::TopLeft;
    handle_cfg.offset        = { track_x, 0.f };
    handle_cfg.panel_size    = { config.scrollbar_width, config.viewport_size.y };
    handle_cfg.panel_color   = config.handle_color;
    m_handle = add_child_entity<Panel>(handle_cfg, "scrollbar-handle");
}

//--------------------------------------------------------------------------------------------------
void ScrollableViewport::on_start()
{
    if (auto handle = m_handle.lock())
    {
        m_handle_transform = handle->get_component<TransformComponent>();
    }

    recompute_scrollbar_layout();
}

//--------------------------------------------------------------------------------------------------
void ScrollableViewport::on_render(SDL_Renderer* renderer)
{
    auto transform = m_transform.lock();
    if (!transform)
    {
        return;
    }

    glm::vec2 const world_pos = transform->get_world_position();
    glm::vec2 const world_size = transform->get_size() * transform->get_world_scale();

    SDL_Rect const self_rect
    {
        .x = static_cast<int>(glm::round(world_pos.x)),
        .y = static_cast<int>(glm::round(world_pos.y)),
        .w = static_cast<int>(glm::round(world_size.x)),
        .h = static_cast<int>(glm::round(world_size.y)),
    };

    m_had_previous_clip = SDL_RenderClipEnabled(renderer);

    SDL_Rect clip_rect = self_rect;
    if (m_had_previous_clip)
    {
        SDL_GetRenderClipRect(renderer, &m_previous_clip_rect);

        SDL_Rect intersected{};
        clip_rect = SDL_GetRectIntersection(&self_rect, &m_previous_clip_rect, &intersected)
            ? intersected
            : SDL_Rect{ 0, 0, 0, 0 };
    }

    SDL_SetRenderClipRect(renderer, &clip_rect);
}

//--------------------------------------------------------------------------------------------------
void ScrollableViewport::on_post_render(SDL_Renderer* renderer)
{
    if (m_had_previous_clip)
    {
        SDL_SetRenderClipRect(renderer, &m_previous_clip_rect);
    }
    else
    {
        SDL_SetRenderClipRect(renderer, nullptr);
    }
}

//--------------------------------------------------------------------------------------------------
void ScrollableViewport::on_event(Event& event)
{
    EventDispatcher dispatcher(event);
    dispatcher.dispatch<MouseMovedEvent>([this](MouseMovedEvent& e){ return this->on_mouse_moved(e); });
    dispatcher.dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent& e){ return this->on_mouse_pressed(e); });
    dispatcher.dispatch<MouseButtonReleasedEvent>([this](MouseButtonReleasedEvent& e){ return this->on_mouse_released(e); });
    dispatcher.dispatch<MouseScrolledEvent>([this](MouseScrolledEvent& e){ return this->on_mouse_scrolled(e); });
}

//--------------------------------------------------------------------------------------------------
void ScrollableViewport::set_content_height(float content_height)
{
    if (content_height == m_content_height)
    {
        return;
    }

    m_content_height = content_height;
    m_scroll_offset = std::clamp(m_scroll_offset, 0.f, get_max_scroll_offset());
    recompute_scrollbar_layout();
}

//--------------------------------------------------------------------------------------------------
void ScrollableViewport::set_scroll_offset(float offset)
{
    float const clamped = std::clamp(offset, 0.f, get_max_scroll_offset());
    if (clamped == m_scroll_offset)
    {
        return;
    }

    m_scroll_offset = clamped;
    recompute_scrollbar_layout();
}

//--------------------------------------------------------------------------------------------------
float ScrollableViewport::get_max_scroll_offset() const
{
    return std::max(0.f, m_content_height - m_viewport_size.y);
}

//--------------------------------------------------------------------------------------------------
bool ScrollableViewport::on_mouse_moved(MouseMovedEvent& event)
{
    m_last_mouse_position = event.get_mouse_position();

    if (m_dragging_handle)
    {
        float const track_height = m_viewport_size.y;
        float const handle_height = std::max(m_min_handle_size, track_height * (m_viewport_size.y / m_content_height));
        float const draggable_range = track_height - handle_height;
        float const max_offset = get_max_scroll_offset();

        if (draggable_range > 0.f && max_offset > 0.f)
        {
            float const mouse_delta = event.get_mouse_y() - m_drag_start_mouse_y;
            float const content_delta = mouse_delta * (max_offset / draggable_range);
            set_scroll_offset(m_drag_start_scroll_offset + content_delta);
        }

        return true;
    }

    bool const was_hovered = m_handle_hovered;
    m_handle_hovered = is_point_within_handle(m_last_mouse_position);
    if (m_handle_hovered != was_hovered)
    {
        resolve_handle_color();
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
bool ScrollableViewport::on_mouse_pressed(MouseButtonPressedEvent& event)
{
    bool const left_mouse_button = event.get_mouse_button() == 1;
    if (left_mouse_button && is_point_within_handle(event.get_click_position()))
    {
        m_dragging_handle = true;
        m_drag_start_mouse_y = event.get_click_y();
        m_drag_start_scroll_offset = m_scroll_offset;
        resolve_handle_color();
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
bool ScrollableViewport::on_mouse_released(MouseButtonReleasedEvent& event)
{
    bool const left_mouse_button = event.get_mouse_button() == 1;
    if (left_mouse_button && m_dragging_handle)
    {
        m_dragging_handle = false;
        resolve_handle_color();
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
bool ScrollableViewport::on_mouse_scrolled(MouseScrolledEvent& event)
{
    if (get_max_scroll_offset() <= 0.f || !is_point_within_viewport(m_last_mouse_position))
    {
        return false;
    }

    set_scroll_offset(m_scroll_offset - event.get_mouse_offset_y() * m_wheel_scroll_step);
    return true;
}

//--------------------------------------------------------------------------------------------------
bool ScrollableViewport::is_point_within_viewport(glm::vec2 const& point) const
{
    auto transform = m_transform.lock();
    if (!transform)
    {
        return false;
    }

    glm::vec2 const top_left = transform->get_world_position();
    glm::vec2 const bottom_right = top_left + (transform->get_size() * transform->get_world_scale());

    return point.x >= top_left.x && point.x <= bottom_right.x
        && point.y >= top_left.y && point.y <= bottom_right.y;
}

//--------------------------------------------------------------------------------------------------
bool ScrollableViewport::is_point_within_handle(glm::vec2 const& point) const
{
    auto transform = m_handle_transform.lock();
    if (!transform)
    {
        return false;
    }

    glm::vec2 const top_left = transform->get_world_position();
    glm::vec2 const bottom_right = top_left + (transform->get_size() * transform->get_world_scale());

    return point.x >= top_left.x && point.x <= bottom_right.x
        && point.y >= top_left.y && point.y <= bottom_right.y;
}

//--------------------------------------------------------------------------------------------------
void ScrollableViewport::recompute_scrollbar_layout()
{
    auto handle = m_handle.lock();
    auto track = m_track.lock();

    bool const scrollable = m_content_height > m_viewport_size.y;
    if (!scrollable)
    {
        if (handle) handle->set_disabled();
        if (track) track->set_disabled();
        return;
    }

    if (handle) handle->set_active();
    if (track) track->set_active();

    float const track_height = m_viewport_size.y;
    float const handle_height = std::max(m_min_handle_size, track_height * (m_viewport_size.y / m_content_height));
    float const max_offset = get_max_scroll_offset();
    float const scroll_ratio = max_offset > 0.f ? (m_scroll_offset / max_offset) : 0.f;
    float const handle_top = scroll_ratio * (track_height - handle_height);
    float const handle_x = m_viewport_size.x - m_scrollbar_width;

    if (auto handle_transform = m_handle_transform.lock())
    {
        handle_transform->set_size({ m_scrollbar_width, handle_height });
    }

    if (handle)
    {
        handle->set_offset({ handle_x, handle_top });
    }
}

//--------------------------------------------------------------------------------------------------
void ScrollableViewport::resolve_handle_color()
{
    if (auto handle = m_handle.lock())
    {
        handle->set_panel_color((m_handle_hovered || m_dragging_handle) ? m_handle_hover_color : m_handle_color);
    }
}

} // namespace Core::UI
