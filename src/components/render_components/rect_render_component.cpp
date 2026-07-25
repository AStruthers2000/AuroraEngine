////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "core/components/render_components/rect_render_component.h"

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
void RectRenderComponent::awake()
{
    m_owning_transform = get_sibling_component<TransformComponent>();
}

//--------------------------------------------------------------------------------------------------
void RectRenderComponent::render(SDL_Renderer* renderer)
{
    if (auto transform = m_owning_transform.lock())
    {
        SDL_FRect dst{
            .x = transform->get_world_position().x,
            .y = transform->get_world_position().y,
            .w = transform->get_world_scale().x,
            .h = transform->get_world_scale().y,
        };

        SDL_SetRenderDrawColor(renderer, m_color.r, m_color.g, m_color.b, m_color.a);
        SDL_RenderFillRect(renderer, &dst);
    }
}

} // namespace Core
