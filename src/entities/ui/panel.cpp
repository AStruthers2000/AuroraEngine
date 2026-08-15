////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "core/entities/ui/panel.h"

#include "core/components/rect_render_component.h"
#include "core/components/transform_component.h"

namespace Core::UI
{

//--------------------------------------------------------------------------------------------------
Panel::Panel(Entity::Owner owner, Configuration const& config)
    : UIElement(owner, config)
    , m_panel_color(config.panel_color)
    , m_has_border(config.has_border)
    , m_border_size(config.border_size)
    , m_border_color(config.border_color)
{
    std::println("{}, {}", config.panel_size.x, config.panel_size.y);
    TransformComponent::Configuration transform_config
    {
        .position = config.offset,
        .size = config.panel_size,
    };

    m_transform = add_component<TransformComponent>("", transform_config);
    m_panel = add_component<RectRenderComponent>("foreground", {.color = config.panel_color});
}

//--------------------------------------------------------------------------------------------------
void Panel::on_render(SDL_Renderer* renderer)
{
    if (m_has_border)
    {
        if (auto transform = m_transform.lock())
        {
            SDL_FRect top, bottom, left, right;

            float panel_width = transform->get_size().x * transform->get_world_scale().x;
            float panel_height = transform->get_size().y * transform->get_world_scale().y;
            glm::vec2 panel_top_left = transform->get_world_position();
            glm::vec2 panel_bottom_left = transform->get_world_position() + glm::vec2{0, panel_height};
            glm::vec2 panel_top_right = transform->get_world_position() + glm::vec2{panel_width, 0};

            top = SDL_FRect
            {
                .x = panel_top_left.x,
                .y = panel_top_left.y,
                .w = panel_width,
                .h = m_border_size.y,
            };
            bottom = SDL_FRect
            {
                .x = panel_bottom_left.x,
                .y = panel_bottom_left.y - m_border_size.y,
                .w = panel_width,
                .h = m_border_size.y,
            };
            left = SDL_FRect
            {
                .x = panel_top_left.x,
                .y = panel_top_left.y,
                .w = m_border_size.x,
                .h = panel_height,
            };
            right = SDL_FRect
            {
                .x = panel_top_right.x - m_border_size.x,
                .y = panel_top_right.y,
                .w = m_border_size.x,
                .h = panel_height,
            };

            SDL_FRect rects[4] = {top, bottom, left, right};
            SDL_SetRenderDrawColor(renderer, m_border_color.r, m_border_color.g, m_border_color.b, m_border_color.a);
            SDL_RenderFillRects(renderer, rects, 4);
        }
    }
}

//--------------------------------------------------------------------------------------------------
void Panel::set_panel_color(SDL_Color const& new_color)
{
    if (auto panel = m_panel.lock())
    {
        panel->set_color(new_color);
    }
}

//--------------------------------------------------------------------------------------------------
void Panel::set_has_border(bool has_border)
{
    m_has_border = has_border;
}

//--------------------------------------------------------------------------------------------------
void Panel::set_border_size(glm::vec2 const& new_border_size)
{
    m_border_size = new_border_size;
}

//--------------------------------------------------------------------------------------------------
void Panel::set_border_color(SDL_Color const& new_color)
{
    m_border_color = new_color;
}


} // namespace Core
