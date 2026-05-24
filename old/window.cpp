#include "core/window.h"

namespace Core
{
Window::Window(WindowSpecification const &spec)
        : m_specification(spec)
{
}

Window::~Window()
{
    destroy();
}

void Window::create()
{
    m_sdl_window = SDL_CreateWindow(m_specification.title.data(),
                                    static_cast<int>(m_specification.window_size.x),
                                    static_cast<int>(m_specification.window_size.y),
                                    SDL_WINDOW_RESIZABLE);
    if (!m_sdl_window)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                 "Error",
                                 "Failed to create window!",
                                 nullptr);
        return;
    }

    m_sdl_renderer = SDL_CreateRenderer(m_sdl_window, nullptr);
    if (!m_sdl_renderer)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                 "Error",
                                 "Failed to create renderer!",
                                 nullptr);
        return;
    }

    set_size(m_specification.window_size);
    SDL_SetRenderLogicalPresentation(m_sdl_renderer,
                                     static_cast<int>(m_specification.logical_size.x),
                                     static_cast<int>(m_specification.logical_size.y),
                                     SDL_RendererLogicalPresentation::SDL_LOGICAL_PRESENTATION_LETTERBOX);

}

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
