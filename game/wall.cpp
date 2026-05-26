#include "wall.h"

Wall::Wall(Core::Layer& owner, std::uint8_t update_order, glm::vec2 position, glm::vec2 scale)
    : Core::Entity(owner, update_order)
    , m_position(position)
    , m_size(scale)
{
    add_component(std::make_unique<Core::TransformComponent>(*this));
}

void Wall::awake_entity()
{
    transform = get_component<Core::TransformComponent>();
    if (transform)
    {
        transform->set_position(m_position);
        transform->set_scale(m_size);
    }
    else
    {
        printf("No transform component found");
    }
}

void Wall::render_entity(SDL_Renderer *renderer)
{
    if (transform)
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
