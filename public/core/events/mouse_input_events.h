////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CORE_EVENTS_MOUSE_INPUT_EVENTS_H
#define CORE_EVENTS_MOUSE_INPUT_EVENTS_H

#include "event.h"

#include <glm/glm.hpp>

#include <format>

namespace Core
{

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////////////////
class MouseMovedEvent : public Event
{
public:
    MouseMovedEvent(float x, float y)
        : m_mouse_position(glm::vec2(x, y)) {}

    inline float get_mouse_x() const { return m_mouse_position.x; }
    inline float get_mouse_y() const { return m_mouse_position.y; }
    inline glm::vec2 get_mouse_position() const { return m_mouse_position; }

    std::string to_string() const override
    {
        return std::format("MouseMovedEvent: ({}, {})", get_mouse_x(), get_mouse_y());
    }

    EVENT_CLASS_TYPE(MouseMoved)
        
private:
    glm::vec2 m_mouse_position{ 0.0, 0.0 };
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////////////////
class MouseScrolledEvent : public Event
{
public:
    MouseScrolledEvent(float x_offset, float y_offset)
        : m_mouse_offset(glm::vec2(x_offset, y_offset)) {}

    inline float get_mouse_offset_x() const { return m_mouse_offset.x; }
    inline float get_mouse_offset_y() const { return m_mouse_offset.y; }
    inline glm::vec2 get_mouse_offset() const { return m_mouse_offset; }

    std::string to_string() const override
    {
        return std::format("MouseScrolledEvent: ({}, {})", get_mouse_offset_x(), get_mouse_offset_y());
    }

    EVENT_CLASS_TYPE(MouseScrolled)
        
private:
    glm::vec2 m_mouse_offset{ 0.0, 0.0 };
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////////////////
class MouseButtonEvent : public Event
{
public:
    inline std::uint8_t get_mouse_button() const { return m_button; }

protected:
    MouseButtonEvent(std::uint8_t button)
        : m_button(button) {}

    std::uint8_t m_button{ 0U };
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////////////////
class MouseButtonPressedEvent : public MouseButtonEvent
{
public:
    MouseButtonPressedEvent(std::uint8_t button, float mouse_x, float mouse_y)
        : MouseButtonEvent(button), m_click_location(glm::vec2(mouse_x, mouse_y)) {}

    inline float get_click_x() const { return m_click_location.x; }
    inline float get_click_y() const { return m_click_location.y; }
    inline glm::vec2 get_click_position() const { return m_click_location; }

    std::string to_string() const override
    {
        return std::format("MouseButtonPressedEvent: {} (location=({}, {}))", m_button, m_click_location.x, m_click_location.y);
    }

    EVENT_CLASS_TYPE(MouseButtonPressed)

private:
    glm::vec2 m_click_location{ 0.0, 0.0 };
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////////////////
class MouseButtonReleasedEvent : public MouseButtonEvent
{
public:
    MouseButtonReleasedEvent(std::uint8_t button)
        : MouseButtonEvent(button) {}

    std::string to_string() const override
    {
        return std::format("MouseButtonReleasedEvent: {}", m_button);
    }

    EVENT_CLASS_TYPE(MouseButtonReleased)
};

} // namespace Core

#endif // CORE_EVENTS_MOUSE_INPUT_EVENTS_H
