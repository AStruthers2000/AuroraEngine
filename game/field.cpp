#include "field.h"

Field::~Field()
{
    if (m_texture)
    {
        SDL_DestroyTexture(m_texture);
        m_texture = nullptr;
    }
}

void Field::initialize()
{
    std::string full_path = std::string(SDL_GetBasePath()) + m_texture_path;

    SDL_Renderer* renderer = get_world().get_engine().get_window().get_sdl_renderer();
    SDL_Surface* surface = IMG_Load(full_path.c_str());
    if (surface)
    {
        m_texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_DestroySurface(surface);
    }
}

void Field::update(float delta_time)
{
}

void Field::render(SDL_Renderer* renderer)
{
    if (m_texture)
    {
        SDL_FRect dst{
            .x = get_transform().get_position().x,
            .y = get_transform().get_position().y,
            .w = m_size.x,
            .h = m_size.y,
        };
        SDL_RenderTexture(renderer, m_texture, nullptr, &dst);
    }
}

void Field::cleanup()
{
    if (m_texture)
    {
        SDL_DestroyTexture(m_texture);
        m_texture = nullptr;
    }
}
