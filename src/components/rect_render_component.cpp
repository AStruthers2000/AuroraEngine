////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "core/components/rect_render_component.h"

#include "core/components/transform_component.h"

namespace Core
{

//--------------------------------------------------------------------------------------------------
RectRenderComponent::RectRenderComponent(Entity& owner, Configuration const& config)
    : RenderComponent(owner, config)
    , m_color(config.color)
{
}

//--------------------------------------------------------------------------------------------------
void RectRenderComponent::on_awake()
{
    m_owning_transform = get_sibling_component<TransformComponent>();
}

//--------------------------------------------------------------------------------------------------
void RectRenderComponent::on_render(SDL_Renderer* renderer)
{
    if (auto transform = m_owning_transform.lock())
    {
        glm::vec2 const drawn_size = transform->get_size() * transform->get_world_scale();
        SDL_FRect dst{
            .x = transform->get_world_position().x,
            .y = transform->get_world_position().y,
            .w = drawn_size.x,
            .h = drawn_size.y,
        };

        SDL_SetRenderDrawColor(renderer, m_color.r, m_color.g, m_color.b, m_color.a);
        SDL_RenderFillRect(renderer, &dst);
    }
}

//--------------------------------------------------------------------------------------------------
void RectRenderComponent::set_color(SDL_Color const& new_color)
{
    m_color = new_color;
}

} // namespace Core
