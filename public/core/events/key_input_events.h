////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2026 AStruthers2000 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CORE_EVENTS_KEY_INPUT_EVENTS_H
#define CORE_EVENTS_KEY_INPUT_EVENTS_H

#include "event.h"

#include <SDL3/SDL.h>

#include <format>
#include <string_view>

namespace std
{

template<>
struct std::formatter<SDL_Scancode> : std::formatter<std::string_view>
{
    auto format(SDL_Scancode scancode, std::format_context& ctx) const
    {
        return std::formatter<std::string_view>::format(SDL_GetScancodeName(scancode), ctx);
    }
};

} // namespace std

namespace Core
{

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////////////////
class KeyEvent : public Event
{
public:
    inline SDL_Scancode get_scancode() const { return m_scancode; }

protected:
    KeyEvent(SDL_Scancode scancode)
        : m_scancode(scancode) {}

    SDL_Scancode m_scancode{ SDL_Scancode::SDL_SCANCODE_UNKNOWN };
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////////////////
class KeyPressedEvent : public KeyEvent
{
public:
    KeyPressedEvent(SDL_Scancode scancode, bool is_repeat)
        : KeyEvent(scancode), m_is_repeat(is_repeat) {}
    
    inline bool is_repeat() const { return m_is_repeat; }

    std::string to_string() const override
    {
        return std::format("KeyPressedEvent: {} (repeat={})", m_scancode, m_is_repeat);
    }

    EVENT_CLASS_TYPE(KeyPressedEvent)

private:
    bool m_is_repeat{ false };
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////////////////
class KeyReleasedEvent : public KeyEvent
{
public:
    KeyReleasedEvent(SDL_Scancode scancode)
        : KeyEvent(scancode) {}

    std::string to_string() const override
    {
        return std::format("KeyReleasedEvent: {}", m_scancode);
    }

    EVENT_CLASS_TYPE(KeyReleasedEvent)
};

} // namespace Core

#endif // CORE_EVENTS_KEY_INPUT_EVENTS_H
