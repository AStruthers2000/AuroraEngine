#include "wall.h"

void Wall::initialize()
{

}

void Wall::update(float delta_time)
{

}

void Wall::render(SDL_Renderer *renderer)
{
    SDL_FRect dst{
        .x = get_transform().get_position().x,
        .y = get_transform().get_position().y,
        .w = get_transform().get_scale().x,
        .h = get_transform().get_scale().y,
    };

    SDL_SetRenderDrawColor(renderer, m_color.r, m_color.g, m_color.b, m_color.a);
    SDL_RenderFillRect(renderer, &dst);
}

void Wall::cleanup()
{

}
