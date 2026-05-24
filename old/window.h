////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ENGINE_WINDOW_H
#define ENGINE_WINDOW_H

#include <glm/glm.hpp>
#include <SDL3/SDL.h>

#include <string_view>

namespace Core
{

struct WindowSpecification
{
    glm::vec2 window_size{1920, 1080};
    glm::vec2 logical_size{680, 384};
    std::string_view title{"Aurora Engine"};
};

class Window
{
public:
    Window(WindowSpecification const& spec);
    ~Window();

    void create();
    void destroy();

    glm::vec2 get_size() const { return m_specification.logical_size; }

    void set_size(glm::vec2 new_size)
    {
        m_specification.window_size = new_size;
    }

    SDL_Window* get_sdl_window() const { return m_sdl_window; }
    SDL_Renderer* get_sdl_renderer() const { return m_sdl_renderer; }

private:
    WindowSpecification m_specification;
    SDL_Window* m_sdl_window = nullptr;
    SDL_Renderer* m_sdl_renderer = nullptr;
};

} // namespace Core

#endif // ENGINE_WINDOW_H
