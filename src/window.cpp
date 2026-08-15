////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "window.h"

namespace Core
{

//--------------------------------------------------------------------------------------------------
Window::Window(WindowSpecification const &spec)
    : m_specification(spec)
{
}

//--------------------------------------------------------------------------------------------------
Window::~Window()
{
    destroy();
}

//--------------------------------------------------------------------------------------------------
void Window::create()
{
    m_sdl_window = SDL_CreateWindow(m_specification.title.c_str(),
                                    static_cast<int>(m_specification.window_size.x),
                                    static_cast<int>(m_specification.window_size.y),
                                    m_specification.sdl_window_flags);
    if (!m_sdl_window)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                 "Error",
                                 "Failed to create window!",
                                 nullptr);
        std::exit(-1);
    }

    m_sdl_renderer = SDL_CreateRenderer(m_sdl_window, nullptr);
    if (!m_sdl_renderer)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                 "Error",
                                 "Failed to create renderer!",
                                 nullptr);
        std::exit(-1);
    }

    SDL_SetRenderDrawBlendMode(m_sdl_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderLogicalPresentation(m_sdl_renderer,
                                     static_cast<int>(m_specification.logical_size.x),
                                     static_cast<int>(m_specification.logical_size.y),
                                     m_specification.sdl_renderer_mode);

    SDL_SetRenderVSync(m_sdl_renderer, m_specification.vsync_flag);
}

//--------------------------------------------------------------------------------------------------
void Window::destroy()
{
    if (m_sdl_renderer)
    {
        SDL_DestroyRenderer(m_sdl_renderer);
        m_sdl_renderer = nullptr;
    }

    if (m_sdl_window)
    {
        SDL_DestroyWindow(m_sdl_window);
        m_sdl_window = nullptr;
    }
}

} // namespace Core
