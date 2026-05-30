#include "wall.h"

Wall::Wall(Core::Layer& owner, std::uint8_t update_order, glm::vec2 position, glm::vec2 scale)
    : Core::Entity(owner, update_order)
{
    m_transform = add_component<Core::TransformComponent>(position, scale);
}

void Wall::render_entity(SDL_Renderer *renderer)
{
    if (auto transform = m_transform.lock())
    {
        SDL_FRect dst{
            .x = transform->get_position().x,
            .y = transform->get_position().y,
            .w = transform->get_scale().x,
            .h = transform->get_scale().y,
        };

        SDL_SetRenderDrawColor(renderer, 125, 175, 225, 255);
        SDL_RenderFillRect(renderer, &dst);
    }
}
