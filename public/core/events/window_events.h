////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CORE_EVENTS_WINDOW_EVENTS_H
#define CORE_EVENTS_WINDOW_EVENTS_H

#include "event.h"

#include <glm/glm.hpp>

#include <format>

namespace Core
{

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////////////////
class WindowCloseEvent : public Event
{
public:
    WindowCloseEvent() {}

    std::string to_string() const override
    {
        return std::format("WindowCloseEvent");
    }

    EVENT_CLASS_TYPE(WindowCloseEvent)
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////////////////
class WindowResizeEvent : public Event
{
public:
    WindowResizeEvent(std::int32_t width, std::int32_t height)
        : m_window_size(glm::vec2(static_cast<float>(width), static_cast<float>(height))) {}

    inline float get_width() const { return m_window_size.x; }
    inline float get_height() const { return m_window_size.y; }
    inline glm::vec2 get_window_size() const { return m_window_size; }

    std::string to_string() const override
    {
        return std::format("WindowResizeEvent: ({}, {})", get_width(), get_height());
    }

    EVENT_CLASS_TYPE(WindowResizeEvent)

private:
    glm::vec2 m_window_size{ 0.0f, 0.0f };
};

} // namespace Core

#endif // CORE_EVENTS_WINDOW_EVENTS_H
